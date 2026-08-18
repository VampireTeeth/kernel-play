//
// Created by steven on 18/8/26.
//

#include "heap.h"
#include "config.h"
#include "memory/memory.h"
#include <stdbool.h>
#include <stdint.h>


static bool validate_alignment(void* ptr)
{
    return (unsigned int)ptr % KERNEL_HEAP_BLOCK_SIZE == 0;
}

static int heap_validate_table(void* start, void* end, heap_table_t* table)
{
    size_t table_size = (size_t)(end - start);
    size_t total_blocks = table_size / KERNEL_HEAP_BLOCK_SIZE;
    if (table->size != total_blocks)
    {
        return -EINVARG;
    }
    return 0;
}

int heap_create(heap_t* heap_ptr, void* start, void* end, heap_table_t* table)
{
    int res = 0;
    if (!validate_alignment(start) || !validate_alignment(end))
    {
        return -EINVARG;
    }
    memset(heap_ptr, 0, sizeof(heap_t));
    heap_ptr->start_addr = start;
    heap_ptr->table = table;
    res = heap_validate_table(start, end, table);
    if (res < 0)
    {
        return res;
    }
    size_t total_bytes = sizeof(heap_block_table_entry_t) * table->size;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE , total_bytes);
    return res;
}

static uint32_t heap_align_to_upper(uint32_t val)
{
    int rem = val % KERNEL_HEAP_BLOCK_SIZE;
    if (rem == 0)
    {
        return val;
    }
    return val + KERNEL_HEAP_BLOCK_SIZE - rem;
}

static bool is_free_block(const heap_block_table_entry_t * const entry)
{
    return (HEAP_BLOCK_TABLE_ENTRY_TAKE & *entry) == 0;
}

static bool is_free_blocks(
    const heap_block_table_entry_t* const cur_start,
    const heap_block_table_entry_t* const cur_end,
    const heap_block_table_entry_t* const end)
{
    if (cur_start >= end)
    {
        return false;
    }
    for (const heap_block_table_entry_t* cur = cur_start; cur < cur_end; cur++)
    {
        if (!is_free_block(cur))
        {
            return false;
        }
    }
    return true;
}

static heap_block_table_entry_t* find_next_free_block(
    heap_block_table_entry_t* const start,
    const heap_block_table_entry_t* const end)
{
    heap_block_table_entry_t* p = start;
    for(;!is_free_block(p) && p < end; p++) {} // find the next free block from start
    return p;
}

static heap_block_table_entry_t* find_free_block_start(const heap_t* const heap, const int blocks)
{
    heap_block_table_entry_t* start = heap->table->entries;
    const heap_block_table_entry_t* const end = heap->table->entries + heap->table->size;
    heap_block_table_entry_t* cur_start = find_next_free_block(start, end);
    heap_block_table_entry_t* cur_end = cur_start + blocks;

    while (cur_start < end)
    {
        if (is_free_blocks(cur_start, cur_end, end))
        {
            return cur_start;
        }
        cur_start = find_next_free_block(cur_start, end);
        cur_end = cur_start + blocks;
    }
    return NULL;
}

static void heap_mark_table_entries_taken(heap_block_table_entry_t* const start, int blocks)
{
    const heap_block_table_entry_t* const end = start + blocks;
    for (heap_block_table_entry_t* cur = start; cur < end; cur++)
    {
        heap_block_table_entry_t new_entry = HEAP_BLOCK_TABLE_ENTRY_TAKE;
        if (cur == start)
        {
            new_entry |= HEAP_BLOCK_IS_FIRST;
        }
        if (cur < end - 1)
        {
            new_entry |= HEAP_BLOCK_HAS_NEXT;
        }
        *cur = new_entry;
    }
}

static void* heap_malloc_blocks(heap_t* heap, int blocks)
{
    // 1. find the suitable pointer in the data pool
    // 2. the suitable pointer should have continuous number of blocks >= `blocks`
    heap_block_table_entry_t* const start = heap->table->entries;

    heap_block_table_entry_t* free_start = find_free_block_start(heap, blocks);
    if (free_start != NULL)
    {
        int block_start = free_start - start;
        heap_mark_table_entries_taken(free_start, blocks);
        // return the pointer in the data pool to the caller to use
        return heap->start_addr + block_start * KERNEL_HEAP_BLOCK_SIZE;
    }
    return NULL;
}

void* heap_malloc(heap_t* heap, size_t size)
{
    int aligned_size = heap_align_to_upper(size);
    int blocks = aligned_size / KERNEL_HEAP_BLOCK_SIZE;
    return heap_malloc_blocks(heap, blocks);
}

void heap_free(heap_t* heap, void* ptr)
{
}