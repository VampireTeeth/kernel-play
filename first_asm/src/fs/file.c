//
// Created by steven on 30/8/26.
//

#include "file.h"
#include "fs/fat/fat16.h"
#include "config.h"
#include "disk/disk.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"

filesystem_t* filesystems[MAX_FILESYSTEMS];
file_descriptor_t* filedescriptors[MAX_FILEDESCRIPTORS];

static filesystem_t** fs_get_free_filesystem()
{
    for (int i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }
    return 0;
}

void fs_insert_filesystem(filesystem_t* filesystem)
{
    filesystem_t** fs = fs_get_free_filesystem();
    if (!fs)
    {
        return;
    }
    *fs = filesystem;
}

static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());
}

static void fs_load()
{
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init()
{
    memset(filedescriptors, 0, sizeof(filedescriptors));
    fs_load();
}

static int file_new_descriptor(file_descriptor_t** fd_out)
{
    int res = -ENOMEM;
    for (int i = 0; i < MAX_FILEDESCRIPTORS; i++)
    {
        if (filedescriptors[i] != 0)
        {
            continue;
        }
        file_descriptor_t* desc = kheap_zalloc(sizeof(file_descriptor_t));
        desc->index = i + 1;
        filedescriptors[i] = desc;
        *fd_out = desc;
        res = 0;
        break;
    }
    return res;
}

static file_descriptor_t* file_get_descriptor(int fd)
{
    if (fd < 0 || fd >= MAX_FILEDESCRIPTORS)
    {
        return 0;
    }
    int index = fd - 1;
    file_descriptor_t* desc = filedescriptors[index];
    return desc;
}

filesystem_t* fs_resolve(struct disk* disk)
{
    filesystem_t* fs = 0;
    for (int i = 0; i < MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }
    return fs;
}
static FILE_MODE get_file_mode(const char* s)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(s, "r", 1) == 0)
    {
        mode = FILE_MODE_READ;
    }
    else if (strncmp(s, "w", 1) == 0)
    {
        mode = FILE_MODE_WRITE;
    }
    else if (strncmp(s, "a", 1) == 0)
    {
        mode = FILE_MODE_APPEND;
    }
    return mode;
}

int fopen(const char* filename, const char* mode_str)
{
    int res = 0;
    path_root_t* path_root = kheap_zalloc(sizeof(path_root_t));
    res = pparser_parse_path_root(filename, path_root);
    if (res < 0)
    {
        goto out;
    }
    if (!path_root->parts)
    {
        res = -EINVARG;
        goto out;
    }
    disk_t* disk = disk_get(path_root->drive_no);
    if (!disk)
    {
        res = -EIO;
        goto out;
    }
    if (!disk->filesystem)
    {
        res = -EIO;
        goto out;
    }
    FILE_MODE mode = get_file_mode(mode_str);
    if (FILE_MODE_INVALID == mode)
    {
        res = -EINVARG;
        goto out;
    }

    void* fd_private = disk->filesystem->open(disk, path_root->parts, mode);
    if (!fd_private)
    {
        res = -EIO;
        goto out;
    }
    if (((int)fd_private) < 0)
    {
        res = (int)fd_private;
        goto out;
    }
    file_descriptor_t* desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0)
    {
        goto out;
    }
    desc->fs = disk->filesystem;
    desc->private = fd_private;
    desc->disk = disk;
    res = desc->index;
out:
    // fopen should not return negative value
    if (res < 0)
    {
        res = 0;
    }
    return res;
}

int fread(void* out, uint32_t size, uint32_t nmemb, int fd)
{
    int res = 0;
    if (size == 0 || nmemb == 0 || fd < 1)
    {
        res = -EINVARG;
        goto out;
    }
    file_descriptor_t* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EINVARG;
        goto out;
    }
    res = desc->fs->read(desc->disk, desc->private, size, nmemb, (char*)out);
    out:
    return res;
}
