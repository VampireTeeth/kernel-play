#include "kernel.h"
#include "io/io.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/pparser.h"

void demo_pparser()
{
    const char* path_str = "0:/test/ok/me.txt";
    const char* path_str1 = "1:/another/dir/you.txt";
    path_root_t* path_root = kheap_zalloc(sizeof(path_root_t));
    int r = pparser_parse_path_root(path_str, path_root);
    if (r < 0)
    {
        print_string("Failed to parse path root!\n");
    }
    pparser_free_path_root(path_root);
    path_root_t* path_root1 = kheap_zalloc(sizeof(path_root_t));
    r = pparser_parse_path_root(path_str1, path_root1);
    if (r < 0)
    {
        print_string("Failed to parse path root!\n");
    }
    pparser_free_path_root(path_root1);
}

void kernel_main() {
    terminal_init();
    print_string("Welcome!\n");
    idtr_init();
    kheap_init();
    disk_search_and_init();
    uint8_t flags = PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT;
    paging_4gb_chunk* chunk = paging_new_4gb(flags);
    paging_directory_entry_t* directory = paging_4gb_chunk_get_directory(chunk);
    paging_switch(directory);
    paging_enable_paging();

    disk_streamer_t* streamer = disk_streamer_new(0);
    disk_streamer_seek_pos(streamer, 0x201);
    char out[1024];
    disk_streamer_read_bytes(streamer, 1, &out);
    disk_streamer_close(streamer);
}
