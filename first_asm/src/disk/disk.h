//
// Created by steven on 22/8/26.
//

#ifndef FIRST_ASM_DISK_H
#define FIRST_ASM_DISK_H
#include "fs/file.h"

typedef unsigned int DISK_TYPE;

// Represents a real physical hard disk
#define DISK_TYPE_REAL 0

typedef struct disk
{
    int id;
    DISK_TYPE type;
    int sector_size;
    filesystem_t* filesystem;
    // private data fon the FS
    void* fs_private;
} disk_t;

int read_from_disk(int lba, int total, void* buf);

void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_sector(disk_t* idisk, unsigned int lba, int total, void* buf);

#endif //FIRST_ASM_DISK_H
