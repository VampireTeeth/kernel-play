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

typedef struct fat_item_descriptor
{
    struct fat_item* item;
    uint32_t pos;
} fat_item_descriptor_t;

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

void* fat16_open(struct disk* disk, path_part_t* path, FILE_MODE mode)
{
    // TODO
    return NULL;
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
    fat_directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
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
