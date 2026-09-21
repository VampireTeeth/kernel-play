//
// Created by steven on 30/8/26.
//

#ifndef FIRST_ASM_FILE_H
#define FIRST_ASM_FILE_H
#include <stdint.h>

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
    FILE_MODE_INVALID,
};

typedef unsigned int FILE_STAT_FLAGS;
enum
{
    FILE_STAT_READ_ONLY,
};


typedef struct file_stat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
} file_stat_t;

struct disk;
typedef void* (*FS_OPEN_FUNCTION)(struct disk* disk, path_part_t* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);
typedef int (*FS_SEEK_FUNCTION)(void* private, int offset, FILE_SEEK_MODE whence);
typedef int (*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int (*FS_STAT_FUNCTION)(struct disk* disk, void* private, file_stat_t* stat);
typedef int (*FS_CLOSE_FUNCTION)(void* private);

typedef struct filesystem
{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_SEEK_FUNCTION seek;
    FS_READ_FUNCTION read;
    FS_STAT_FUNCTION stat;
    FS_CLOSE_FUNCTION close;
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
void fs_insert_filesystem(filesystem_t* filesystem);

int fopen(const char* filename, const char* mode);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
int fread(void* out, uint32_t size, uint32_t nmemb, int fd);
int fstat(int fd, file_stat_t* stat);
int fclose(int fd);

filesystem_t* fs_resolve(struct disk* disk);
#endif //FIRST_ASM_FILE_H
