//
// Created by steven on 22/8/26.
//
#include "disk.h"

#include "config.h"
#include "fs/file.h"
#include "io/io.h"

#include "memory/memory.h"

disk_t root_disk;

static int read_from_disk(int lba, int total, void* buf)
{
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char) lba & 0xFF);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);
    unsigned short* ptr = buf;
    for (int i = 0; i < total; i++)
    {
        char c = insb(0x1F7);
        while (!(c & 0x08))
        {
            c = insb(0x1F7);
        }

        int max_words = DISK_SECTOR_SIZE / 2;
        for (int j = 0; j < max_words; j++)
        {
            *ptr = insw(0x1F0);
            ptr++;
        }
    }

    return 0;
}

void disk_search_and_init()
{
    memset(&root_disk, 0, sizeof(disk_t));
    root_disk.type = DISK_TYPE_REAL;
    root_disk.sector_size = DISK_SECTOR_SIZE;
    root_disk.filesystem = fs_resolve(&root_disk);
}

disk_t* disk_get(int index)
{
    if (index != 0)
        return 0;

    return &root_disk;
}

int disk_read_sector(disk_t* idisk, unsigned int lba, int total, void* buf)
{
    if (idisk != &root_disk)
    {
        return -EIO;
    }

    return read_from_disk(lba, total, buf);
}
