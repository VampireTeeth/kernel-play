#include "kernel.h"
#include "io/io.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "disk/disk.h"
#include "fs/pparser.h"

void kernel_main() {
    terminal_init();
    print_string("Welcome!\n");
    disk_search_and_init();
    idtr_init();
    kheap_init();
    uint8_t flags = PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT;
    paging_4gb_chunk* chunk = paging_new_4gb(flags);
    paging_directory_entry_t* directory = paging_4gb_chunk_get_directory(chunk);
    void* ptr1 = kheap_zalloc(5000);
    paging_switch(directory);
    paging_enable_paging();

    paging_set_value(directory, (void*)0x10000, (uint32_t)ptr1 | flags);
    char* ptr2 = (void*) 0x10000;
    ptr2[0] = 'H';
    ptr2[1] = 'A';

    print_string(ptr2);
    print_string(ptr1);


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
