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

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

#include <stdint.h>

typedef struct paging_4gb_chunk
{
    uint32_t* directory;
} paging_4gb_chunk;

#endif //FIRST_ASM_PAGING_H
