//
// Created by steven on 7/9/26.
//
#include "fat16.h"
#include "config.h"
#include "fs/file.h"
#include "string/string.h"
#include "disk/streamer.h"
#include <stdint.h>

#include "memory/memory.h"
#include "memory/heap/kheap.h"

#define KERNEL_FAT16_SIGNATURE 0x29
#define KERNEL_FAT16_FAT_ENTRY_SIZE 0x02
#define KERNEL_FAT16_BAD_SECTOR 0xFF7
#define KERNEL_FAT16_UNUSED 0x00


typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80

struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

typedef struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended extended_header;
    } shared;
} fat_h_t;

struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

typedef struct fat_directory
{
    struct fat_directory_item* item;
    int total;
    int sector_pos;
    int ending_sector_pos;
} fat_directory_t;

typedef struct fat_item
{
    union
    {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };

    FAT_ITEM_TYPE type;
} fat_item_t;

typedef struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t pos;
} fat_file_descriptor_t;

typedef struct fat_private
{
    fat_h_t header;
    fat_directory_t root_directory;

    // Used to stream data clusters
    disk_streamer_t* cluster_read_stream;
    // Used to stream the file allocation table
    disk_streamer_t* fat_read_stream;

    // Used in situations where we stream the directory
    disk_streamer_t* directory_stream;
} fat_private_t;

void* fat16_open(struct disk* disk, path_part_t* path, FILE_MODE mode);

int fat16_resolve(struct disk* disk);

filesystem_t fat16_fs =
{
    .resolve = fat16_resolve,
    .open = fat16_open
};

filesystem_t* fat16_init()
{
    strcpy(fat16_fs.name,  "FAT16");
    return &fat16_fs;
}

static int fat16_get_total_items_for_directory(disk_t* disk, fat_private_t* fat_private, int dir_sector_pos);

static void fat16_to_proper_string(char** out, const char* in)
{
    while (in != 0x00 && *in != 0x20)
    {
        **out = *in;
        *out += 1;
        in += 1;
    }
    if (*in == 0x20)
    {
        **out = 0x00;
    }
}

static void fat16_get_full_relative_filename(const struct fat_directory_item* item, char* out, int len)
{
    memset(out, 0x00, len);
    char* out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*)item->filename);
    if (item->ext[0] != 0x00 && item->ext[0] != 0x20)
    {
        *out_tmp = '.';
        *out_tmp += 1;
        fat16_to_proper_string(&out_tmp, (const char*)item->ext);
    }
}

static struct fat_directory_item* fat16_clone_directory_item(struct fat_directory_item* item)
{
    size_t size = sizeof(struct fat_directory_item);
    struct fat_directory_item* item_copy = kheap_zalloc(size);
    if (!item_copy)
    {
        return 0;
    }
    memcpy(item_copy, item, size);
    return item_copy;
}

int fat16_cluster_to_sector(fat_private_t* fat_private, int cluster)
{
    int sectors_per_cluster = fat_private->header.primary_header.sectors_per_cluster;
    return fat_private->root_directory.ending_sector_pos + ((cluster - 2) * sectors_per_cluster);
}

static int fat16_get_first_cluster(struct fat_directory_item* item)
{
    return ((int)item->high_16_bits_first_cluster << 16) | (int)item->low_16_bits_first_cluster;
}

static int fat16_get_cluster_size_in_bytes(disk_t* disk, fat_private_t* fat_private)
{
    return fat_private->header.primary_header.sectors_per_cluster * disk->sector_size;
}

static uint32_t fat16_get_first_fat_sector(fat_private_t* fat_private)
{
    return fat_private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(disk_t* disk, int cluster)
{
    int res = 0;
    fat_private_t* fat_private = disk->fs_private;
    disk_streamer_t* stream = fat_private->fat_read_stream;
    if (!stream)
    {
        goto out;
    }
    uint32_t fat_table_position = fat16_get_first_fat_sector(fat_private) * disk->sector_size;
    res = disk_streamer_seek_pos(stream, fat_table_position * (cluster * KERNEL_FAT16_FAT_ENTRY_SIZE));
    if (res < 0)
    {
        goto out;
    }
    uint16_t result = 0;
    disk_streamer_read_bytes(stream, sizeof(result), &result);
    if (res < 0)
    {
        goto out;
    }
    res = result;
    out:
    return res;
}

static int fat16_get_cluster_for_offset(disk_t* disk, int cluster, int offset)
{
    int res = 0;
    fat_private_t* fat_private = disk->fs_private;
    int cluster_bytes = fat16_get_cluster_size_in_bytes(disk, fat_private);
    int cluster_to_use = cluster;
    int clusters_ahead = offset / cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        if (entry == 0xFF8 || entry == 0xFFF)
        {
            // We are at the last entry in the file
            res = -EIO;
            goto out;
        }
        // Sector is marked as bad
        if (entry == KERNEL_FAT16_BAD_SECTOR)
        {
            res = -EIO;
            goto out;
        }
        // Sector is reserved
        if (entry == 0xFF0 || entry == 0xFF6)
        {
            res = -EIO;
            goto out;
        }
        if (entry == 0x00)
        {
            res = -EIO;
            goto out;
        }
        cluster_to_use = entry;
    }
    res = cluster_to_use;
    out:
    return res;
}

static int fat16_read_internal_from_stream(disk_t* disk, disk_streamer_t* stream, int cluster, int offset, int total, void* out)
{
    int res = 0;
    fat_private_t* fat_private = disk->fs_private;
    int size_of_cluster_bytes = fat16_get_cluster_size_in_bytes(disk, fat_private);
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (!cluster_to_use)
    {
        res = cluster_to_use;
        goto out;
    }
    int offset_from_cluster = offset % size_of_cluster_bytes;
    int starting_sector = fat16_cluster_to_sector(fat_private, cluster);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
    int total_to_read = total > size_of_cluster_bytes ? size_of_cluster_bytes : total;
    res = disk_streamer_seek_pos(stream, starting_pos);
    if (res != OK)
    {
        goto out;
    }
    res = disk_streamer_read_bytes(stream, total_to_read, out);
    if (res != OK)
    {
        goto out;
    }
    total -= total_to_read;
    if (total > 0)
    {
        res = fat16_read_internal_from_stream(disk, stream, cluster, offset+total_to_read, total, out + total_to_read);
    }
    out:
    return res;
}

int fat16_read_internal(disk_t* disk, int starting_cluster, int offset, int total, void* out)
{
    fat_private_t* fat_private = disk->fs_private;
    disk_streamer_t* stream = fat_private->cluster_read_stream;
    int res = fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
    return res;
}

static void fat16_directory_free(fat_directory_t* directory)
{
    if (!directory)
    {
        return;
    }
    if (directory->item)
    {
        kheap_free(directory->item);
    }
    kheap_free(directory);
}

static void fat16_fat_item_free(fat_item_t* fat_item)
{
    if (fat_item->type == FAT_ITEM_TYPE_DIRECTORY)
    {
        fat16_directory_free(fat_item->directory);
    }
    else if (fat_item->type == FAT_ITEM_TYPE_FILE)
    {
        kheap_free(fat_item->item);
    }
    kheap_free(fat_item);
}

struct fat_directory* fat16_load_fat_directory(disk_t* disk, struct fat_directory_item* item)
{
    int res = 0;
    fat_directory_t* directory = 0;
    fat_private_t* fat_private = disk->fs_private;
    if (!(item->attribute & FAT_FILE_SUBDIRECTORY))
    {
        res = -EINVARG;
        goto out;
    }
    directory = kheap_zalloc(sizeof(fat_directory_t));
    if (!directory)
    {
        res = -ENOMEM;
        goto out;
    }
    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    int total_items = fat16_get_total_items_for_directory(disk, fat_private, cluster_sector);
    directory->total = total_items;
    int directory_size = directory->total * sizeof(struct fat_directory_item*);
    directory->item = kheap_zalloc(directory_size);
    if (!directory->item)
    {
        res = -ENOMEM;
        goto out;
    }
    res = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->item);
    if (res != OK)
    {
        goto out;
    }
    out:
    if (res != OK)
    {
        fat16_directory_free(directory);
    }
    return directory;
}

// create a new fat_item_t* for a given fat_directory_item* on the given disk
// if the given directory item is a directory, then load the fat_item_t* as fat_directory* (directory)
// otherwise load the fat_item_t* as fat_directory_item* (file)
static fat_item_t* fat16_new_fat_item_for_directory_item(disk_t* disk, struct fat_directory_item* directory_item)
{
    fat_item_t* item = kheap_zalloc(sizeof(fat_item_t));
    if (!item)
    {
        return 0;
    }
    if (directory_item->attribute & FAT_FILE_SUBDIRECTORY)
    {
        item->directory = fat16_load_fat_directory(disk, directory_item);
        item->type = FAT_ITEM_TYPE_DIRECTORY;
        goto out;
    }
    item->type = FAT_ITEM_TYPE_FILE;
    item->item = fat16_clone_directory_item(directory_item);
    out:
    return item;
}

// find the fat_item_t* that matches the given file name
static fat_item_t* fat16_get_item_in_directory(disk_t* disk, const fat_directory_t* directory, const char* path)
{
    fat_item_t* fat_item = 0;
    char tmp_filename[PPARSER_MAX_PATH];
    for (int i = 0; i < directory->total; i++)
    {
        fat16_get_full_relative_filename(&directory->item[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, path, sizeof(tmp_filename)) == 0)
        {
            // found it: create a new fat item
            fat_item = fat16_new_fat_item_for_directory_item(disk, &directory->item[i]);
            break;
        }
    }
    return fat_item;
}


static fat_item_t* fat16_get_directory_entry(disk_t* disk, path_part_t* path)
{
    fat_private_t* fat_private = disk->fs_private;
    fat_item_t* current_item = 0;
    fat_item_t* root_item = fat16_get_item_in_directory(disk, &fat_private->root_directory, path->path);
    if (!root_item)
    {
        goto out;
    }
    current_item = root_item;
    path_part_t* next_part = path->next;
    while (next_part != 0)
    {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY)
        {
            // Invalid state, we are trying to accessing a file when it also has next path
            current_item = 0;
            break;
        }
        fat_item_t* tmp_item = fat16_get_item_in_directory(disk, current_item->directory, next_part->path);
        fat16_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
    out:
    return current_item;
}

void* fat16_open(struct disk* disk, path_part_t* path, FILE_MODE mode)
{
    if (mode != FILE_MODE_READ)
    {
        return (void*)(-ERDONLY);
    }
    fat_file_descriptor_t* descriptor = 0;
    descriptor = kheap_zalloc(sizeof(fat_file_descriptor_t));
    if (!descriptor)
    {
        return (void*)(-ENOMEM);
    }
    fat_item_t* entry = fat16_get_directory_entry(disk, path);
    if (!entry)
    {
        return (void*)(-EIO);
    }
    descriptor->item = entry;
    descriptor->pos = 0;
    return descriptor;
}

static int fat16_init_private(fat_private_t* fat_private, struct disk* disk)
{
    memset(fat_private, 0, sizeof(fat_private_t));
    int disk_id = disk->id;
    fat_private->cluster_read_stream = disk_streamer_new(disk_id);
    if (!fat_private->cluster_read_stream)
    {
        return -EIO;
    }
    fat_private->fat_read_stream = disk_streamer_new(disk_id);
    if (!fat_private->fat_read_stream)
    {
        return -EIO;
    }
    fat_private->directory_stream = disk_streamer_new(disk_id);
    if (!fat_private->directory_stream)
    {
        return -EIO;
    }
    return 0;
}

static int sector_pos_to_bytes_offset(disk_t* disk, int sector_pos)
{
    return disk->sector_size * sector_pos;
}

static int fat16_get_total_items_for_directory(disk_t* disk, fat_private_t* fat_private, int dir_sector_pos)
{
    struct fat_directory_item item;
    memset(&item, 0, sizeof(item));
    int res = 0;
    int count = 0;
    int dir_bytes_offset = sector_pos_to_bytes_offset(disk, dir_sector_pos);
    disk_streamer_t* ds = fat_private->directory_stream;
    if (disk_streamer_seek_pos(ds, dir_bytes_offset) != OK)
    {
        res = -EIO;
        goto out;
    }
    while(1)
    {
        if (disk_streamer_read_bytes(ds, sizeof(item), &item) != OK)
        {
            res = -EIO;
            goto out;
        }
        if (item.filename[0] == 0x00)
        {
            // we are done
            break;
        }
        if (item.filename[0] == 0xE5)
        {
            // item is unused
            continue;
        }
        count++;
    }
    res = count;
    out:
    return res;
}

static int fat16_get_root_directory(disk_t* disk, fat_private_t* fat_private, fat_directory_t* fat_directory)
{
    struct fat_header* primary_header = &fat_private->header.primary_header;
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_entries = primary_header->root_dir_entries;
    int root_dir_size = root_dir_entries * sizeof(struct fat_directory_item);
    int total_sectors = root_dir_size / disk->sector_size;
    if (root_dir_size % disk->sector_size)
    {
        total_sectors += 1;
    }
    int total_items = fat16_get_total_items_for_directory(disk, fat_private, root_dir_sector_pos);
    if (total_items < 0)
    {
        return -EIO;
    }
    struct fat_directory_item* root_dir_items = kheap_zalloc(root_dir_size);
    if (!root_dir_items)
    {
        return -ENOMEM;
    }
    disk_streamer_t* ds = fat_private->directory_stream;
    int res = disk_streamer_seek_pos(ds, sector_pos_to_bytes_offset(disk, root_dir_sector_pos));
    if (res != OK)
    {
        return -EIO;
    }
    res = disk_streamer_read_bytes(ds, root_dir_size, root_dir_items);
    if (res != OK)
    {
        return -EIO;
    }
    fat_directory->item = root_dir_items;
    fat_directory->sector_pos = root_dir_sector_pos;
    fat_directory->total = total_items;
    fat_directory->ending_sector_pos = root_dir_sector_pos + total_sectors; // This is the end of the last sector
    return 0;
}

int fat16_resolve(struct disk* disk)
{
    int res = 0;
    fat_private_t* fat_private = kheap_zalloc(sizeof(fat_private_t));
    if (!fat_private)
    {
        res = -ENOMEM;
        goto out;
    }
    res = fat16_init_private(fat_private, disk);
    if (res != OK)
    {
        goto out;
    }
    disk_streamer_t* disk_stream = disk_streamer_new(disk->id);
    if (!disk_stream)
    {
        res = -EIO;
        goto out;
    }
    res = disk_streamer_read_bytes(disk_stream, sizeof(fat_private->header), &fat_private->header);
    if (res != OK)
    {
        res = -EIO;
        goto out;
    }
    if (fat_private->header.shared.extended_header.signature != KERNEL_FAT16_SIGNATURE)
    {
        res = -EFSNOTUS;
        goto out;
    }
    res = fat16_get_root_directory(disk, fat_private, &fat_private->root_directory);
    if (res != OK) {
        res = -EIO;
        goto out;
    }
    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;

    out:
    if (disk_stream)
    {
        disk_streamer_close(disk_stream);
    }
    if (res < 0)
    {
        kheap_free(fat_private);
        disk->fs_private = NULL;
    }
    return res;
}
