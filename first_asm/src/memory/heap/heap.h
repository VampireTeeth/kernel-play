//
// Created by steven on 18/8/26.
//

#ifndef FIRST_ASM_HEAP_H
#define FIRST_ASM_HEAP_H

#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKE 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00
#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST 0b01000000

// heap_block_table_entry_t is one byte data
// that contains the metadata of a block
//
//
// 7: 1 has_next
// 6: 1 is_first
// 5: 0 not used
// 4: type attributes
// 3: type attributes
// 2: type attributes
// 1: type attributes
// 0: 1 taken
typedef unsigned char heap_block_table_entry_t;

typedef struct heap_table
{
    heap_block_table_entry_t* entries;
    size_t size; // number of total blocks in the table
} heap_table_t;

typedef struct heap
{
    heap_table_t* table;
    void* start_addr; // Start of the memory address of the heap
} heap_t;

int heap_create(
    heap_t* heap_ptr, heap_table_t* table,
    void* heap_start, void* table_start,
    size_t heap_size, size_t block_size);

void* heap_malloc(heap_t* heap, size_t size);

void heap_free(heap_t* heap, void* ptr);

#endif //FIRST_ASM_HEAP_H
