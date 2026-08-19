//
// Created by steven on 18/8/26.
//

#include "kheap.h"
#include "config.h"
#include "memory/memory.h"
#include "heap.h"
#include "io/io.h"

heap_t heap;
heap_table_t heap_table;
//for a 100MB heap: 1024 * 1024 * 100 / 4096 = 25600
//this is the number entries of heap_table
void kheap_init()
{
    int heap_table_size = KERNEL_HEAP_SIZE_IN_BYTES / KERNEL_HEAP_BLOCK_SIZE;
    heap_table.size = heap_table_size;
    heap_block_table_entry_t* entries = (heap_block_table_entry_t*) (KERNEL_HEAP_TABLE_ADDRESS);
    memset(entries, 0, sizeof(heap_block_table_entry_t) * heap_table_size);
    heap_table.entries = entries;
    void* end = (void*) (KERNEL_HEAP_ADDRESS + KERNEL_HEAP_SIZE_IN_BYTES);
    int res = heap_create(&heap, (void*)KERNEL_HEAP_ADDRESS, end, &heap_table);
    if (res < 0)
    {
        print_string("Failed to create kernel heap\n");
    }
}

void* kheap_malloc(size_t size)
{
    return heap_malloc(&heap, size);
}

void kheap_free(void* ptr)
{
    heap_free(&heap, ptr);
}