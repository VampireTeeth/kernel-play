#include "kernel.h"
#include "io/io.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"

void kernel_main() {
    terminal_init();
    idtr_init();
    kheap_init();

    paging_4gb_chunk* chunk = paging_new_4gb(PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT);
    paging_directory_entry_t* directory = paging_4gb_chunk_get_directory(chunk);
    void* addr = kheap_zalloc(5000);
    paging_setup_paging_for_address(chunk, addr);
    paging_switch(directory);
    paging_enable_paging();
}