//
// Created by steven on 27/8/26.
//

#ifndef FIRST_ASM_STREAMER_H
#define FIRST_ASM_STREAMER_H
#include "disk.h"

typedef struct disk_streamer
{
    int pos;
    disk_t* disk;
} disk_streamer_t;

disk_streamer_t* disk_streamer_new(int disk_id);

int disk_streamer_seek_pos(disk_streamer_t* streamer, int pos);

int disk_streamer_read_bytes(disk_streamer_t* streamer, int total, void* out);

void disk_streamer_close(disk_streamer_t* streamer);
#endif //FIRST_ASM_STREAMER_H
