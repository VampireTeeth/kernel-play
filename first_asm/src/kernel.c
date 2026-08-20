#include "kernel.h"
#include "io/io.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"

void kernel_main() {
    terminal_init();
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
}