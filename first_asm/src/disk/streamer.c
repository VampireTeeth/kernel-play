//
// Created by steven on 27/8/26.
//

#include "streamer.h"

#include "config.h"
#include "memory/heap/kheap.h"

disk_streamer_t* disk_streamer_new(int disk_id)
{
    disk_t* disk = disk_get(disk_id);
    if (!disk)
    {
        return NULL;
    }
    disk_streamer_t* streamer = kheap_zalloc(sizeof(disk_streamer_t));
    streamer->pos = 0;
    streamer->disk = disk;
    return streamer;
}

int disk_streamer_seek_pos(disk_streamer_t* streamer, int pos)
{
    streamer->pos = pos;
    return 0;
}

int disk_streamer_read_bytes(disk_streamer_t* streamer, int total, void* out)
{
    int res = 0;
    if (total <= 0 )
    {
        return res;
    }
    int sector = streamer->pos / DISK_SECTOR_SIZE;
    int offset = streamer->pos % DISK_SECTOR_SIZE;
    int max_readable = DISK_SECTOR_SIZE - offset;
    char buf[max_readable];
    res = disk_read_sector(streamer->disk, sector, 1, buf);
    if (!res)
    {
        return res;
    }
    int total_to_read = total > max_readable ? max_readable: total;
    for (int i = 0; i < total_to_read; i++)
    {
        *(char*)out = buf[offset+i];
        out++;
    }
    streamer->pos += total_to_read;
    if (total > max_readable)
    {
        res = disk_streamer_read_bytes(streamer, total - total_to_read, out);
    }
    return res;
}

void disk_streamer_close(disk_streamer_t* streamer)
{
    kheap_free(streamer);
}