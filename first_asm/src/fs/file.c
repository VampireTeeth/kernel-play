//
// Created by steven on 30/8/26.
//

#include "file.h"

#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"

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
    // fs_insert_filesystem(fat16_init());
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

static int fs_new_descriptor(file_descriptor_t** fd_out)
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

static file_descriptor_t* fs_get_file_descriptor(int fd)
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

int fopen(const char* filename, const char* mode)
{
    return -EIO;
}
