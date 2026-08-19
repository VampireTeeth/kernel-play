#include "kernel.h"
#include "io/io.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"

void kernel_main() {
    const char hello[] = "Hello, my kernel!";
    terminal_init();
    print_string(hello);

    idtr_init();
    kheap_init();
    void* ptr1 = kheap_malloc(50);
    void* ptr2 = kheap_malloc(15000);
    kheap_free(ptr2);
    void* ptr3 = kheap_malloc(5000);
    kheap_free(ptr3);
    kheap_free(ptr1);
}