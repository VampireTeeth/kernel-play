//
// Created by steven on 22/8/26.
//
#include "disk.h"

#include "config.h"
#include "io/io.h"

#include "memory/memory.h"

disk_t disk;

static int read_from_disk(int lda, int total, void* buf)
{
    outb(0x1F6, (lda >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, lda & 0xFF);
    outb(0x1F4, (lda >> 8) & 0xFF);
    outb(0x1F5, (lda >> 16) & 0xFF);
    outb(0x1F7, 0x20);
    unsigned short* ptr = buf;;
    for (int i = 0; i < total; i++)
    {
        char c = insb(0x1F7);
        while (!(c & 0x08))
        {
            c = insb(0x1F7);
        }
        const unsigned short b = insw(0x1F0);
        ptr[i] = b;
    }

    return 0;
}

void disk_search_and_init()
{
    memset(&disk, 0, sizeof(disk));
    disk.type = DISK_TYPE_REAL;
    disk.sector_size = DISK_SECTOR_SIZE;
}

disk_t* disk_get(int index)
{
    if (index != 0)
        return 0;

    return &disk;
}

int disk_read_block(disk_t* idisk, unsigned int lba, int total, void* buf)
{
    if (idisk != &disk)
    {
        return -EIO;
    }

    return read_from_disk(lba, total, buf);
}
