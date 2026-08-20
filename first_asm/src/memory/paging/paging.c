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
            table[j] = (paging_table_entry_t)((offset + j * PAGING_PAGE_SIZE) | flags);
        }
        offset += PAGING_PAGE_SIZE * PAGING_TOTAL_ENTRIES_PER_TABLE;
        directory[i] = (paging_directory_entry_t)((uint32_t)table | flags | PAGING_IS_WRITABLE);
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

static void paging_mask_table_entry(paging_table_entry_t* table_entry, uint32_t mask)
{
    uint32_t value = (uint32_t)*table_entry;
    *table_entry = (paging_table_entry_t) (value | mask);
}

static void paging_mask_directory_entry(paging_directory_entry_t* directory_entry, uint32_t mask)
{
    uint32_t value = (uint32_t)*directory_entry;
    *directory_entry = (paging_directory_entry_t) (value | mask);
}

int paging_find_directory_and_table_index(paging_directory_entry_t* directory, void* address, paging_directory_entry_t** table_ptr, paging_table_entry_t** entry_ptr)
{
    if (!paging_is_address_aligned(address))
    {
        return -EINVARG;
    }
    uint32_t table_total = PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
    *table_ptr = directory + ((uint32_t)address / table_total);
    *entry_ptr = **table_ptr + ((uint32_t) address % table_total / PAGING_PAGE_SIZE);
    return 0;
}

int paging_setup_paging_for_address(paging_4gb_chunk* chunk, void* address)
{
    if (!paging_is_address_aligned(address))
    {
        return -EINVARG;
    }
    paging_directory_entry_t* table_ptr = 0;
    paging_table_entry_t* entry_ptr = 0;
    int r = paging_find_directory_and_table_index(chunk->directory, address, &table_ptr, &entry_ptr);
    if (r < 0)
    {
        return r;
    }
    uint32_t address_mask = PAGING_ADDRESS_MASK;
    paging_mask_directory_entry(table_ptr, address_mask);
    paging_mask_table_entry(entry_ptr, address_mask);
    return 0;
}