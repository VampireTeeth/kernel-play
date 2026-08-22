//
// Created by steven on 22/8/26.
//

#ifndef FIRST_ASM_DISK_H
#define FIRST_ASM_DISK_H

typedef unsigned int DISK_TYPE;

// Represents a real physical hard disk
#define DISK_TYPE_REAL 0

typedef struct disk
{
    DISK_TYPE type;
    int sector_size;
} disk_t;

void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(disk_t* idisk, unsigned int lba, int total, void* buf);

#endif //FIRST_ASM_DISK_H
