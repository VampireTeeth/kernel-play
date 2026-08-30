//
// Created by steven on 30/8/26.
//

#ifndef FIRST_ASM_FILE_H
#define FIRST_ASM_FILE_H
#include "pparser.h"

typedef unsigned int FILE_SEEK_MODE;
enum
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum
{
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

struct disk;
typedef void* (*FS_OPEN_FUNCTION)(struct disk* disk, path_part_t* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

typedef struct filesystem
{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    char name[20];
} filesystem_t;

typedef struct file_descriptor
{
    int index;
    filesystem_t* fs;

    void* private;

    struct disk* disk;
} file_descriptor_t;

void fs_init();
int fopen(const char* filename, const char* mode);
void fs_insert_filesystem(filesystem_t* filesystem);
filesystem_t* fs_resolve(struct disk* disk);
#endif //FIRST_ASM_FILE_H
