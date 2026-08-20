//
// Created by steven on 19/8/26.
//
// https://wiki.osdev.org/X86_Paging#Page_Directory for more details
#include "paging.h"

#include "memory/heap/kheap.h"
#include <stdbool.h>

#include "config.h"

paging_directory_entry_t* current_directory = NULL;

paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    paging_directory_entry_t* directory = kheap_zalloc(PAGING_TOTAL_ENTRIES_PER_TABLE * sizeof(paging_directory_entry_t));
    uint32_t offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        paging_table_entry_t* table = kheap_zalloc(PAGING_TOTAL_ENTRIES_PER_TABLE * sizeof(paging_table_entry_t));
        for (int j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++)
        {
            table[j] = (offset + j * PAGING_PAGE_SIZE) | flags;
        }
        offset += PAGING_PAGE_SIZE * PAGING_TOTAL_ENTRIES_PER_TABLE;
        directory[i] = (uint32_t)table | flags | PAGING_IS_WRITABLE;
    }
    paging_4gb_chunk* page_chunk = kheap_zalloc(sizeof (paging_4gb_chunk));
    page_chunk->directory = directory;
    return page_chunk;
}

paging_directory_entry_t* paging_4gb_chunk_get_directory(const paging_4gb_chunk* chunk)
{
    return chunk->directory;
}

void paging_switch(paging_directory_entry_t* directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

static bool paging_is_address_aligned(void* address)
{
    return ((uint32_t) address % PAGING_PAGE_SIZE) == 0;
}

int paging_set_value(paging_directory_entry_t* directory, void* virtual_address, uint32_t table_entry)
{
    if (!paging_is_address_aligned(virtual_address))
    {
        return -EINVARG;
    }

    uint32_t addr = (uint32_t) virtual_address;
    uint32_t table_total = PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
    paging_directory_entry_t directory_entry = directory[addr / table_total];
    paging_table_entry_t* table = (paging_table_entry_t*)(directory_entry & PAGING_ADDRESS_MASK);
    table[addr % table_total / PAGING_PAGE_SIZE] = table_entry;
    return 0;
}