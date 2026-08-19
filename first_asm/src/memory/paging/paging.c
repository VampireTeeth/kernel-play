//
// Created by steven on 19/8/26.
//
// https://wiki.osdev.org/X86_Paging#Page_Directory for more details
#include "paging.h"

#include "memory/heap/kheap.h"

paging_4gb_chunk* paging_new_4gb(uint8_t flags)
{
    uint32_t* directory = kheap_zalloc(PAGING_TOTAL_ENTRIES_PER_TABLE * sizeof(uint32_t));
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        uint32_t* table = kheap_zalloc(PAGING_TOTAL_ENTRIES_PER_TABLE * sizeof(uint32_t));
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
