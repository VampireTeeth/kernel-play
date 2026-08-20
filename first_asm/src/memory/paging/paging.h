//
// Created by steven on 19/8/26.
//

#ifndef FIRST_ASM_PAGING_H
#define FIRST_ASM_PAGING_H

#define PAGING_CACHE_DISABLED  0b00010000
#define PAGING_WRITE_THROUGH   0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_WRITABLE     0b00000010
#define PAGING_IS_PRESENT      0b00000001
#define PAGING_ADDRESS_MASK    0xfffff000 // we only need the higher 20 bits of the virtual address because of 4096 alignment

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

#include <stdint.h>

typedef uint32_t paging_table_entry_t;
typedef uint32_t paging_directory_entry_t;

typedef struct paging_4gb_chunk
{
    paging_directory_entry_t* directory;
} paging_4gb_chunk;

paging_4gb_chunk* paging_new_4gb(uint8_t flags);

paging_directory_entry_t* paging_4gb_chunk_get_directory(const paging_4gb_chunk* chunk);

void paging_switch(paging_directory_entry_t* directory);

extern void paging_load_directory(paging_directory_entry_t* directory);

extern void paging_enable_paging();

int paging_set_value(paging_directory_entry_t* directory, void* virtual_address, uint32_t table_entry);
#endif //FIRST_ASM_PAGING_H
