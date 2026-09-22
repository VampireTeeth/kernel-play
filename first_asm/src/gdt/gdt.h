//
// Created by steven on 22/9/26.
//

#ifndef FIRST_ASM_GDT_H
#define FIRST_ASM_GDT_H
#include <stdint.h>

typedef struct gdt
{
    uint16_t limit_first; // limit: 16-bits
    uint16_t base_first;
    uint8_t base;
    uint8_t access_type;
    uint8_t flags_and_limit; // flags: high 4-bits, limit: low 4-bits
    uint8_t base_24_31_bits;
} __attribute__((packed)) gdt_t;

typedef struct gdt_structured
{
    uint32_t base;
    uint32_t limit;
    uint8_t access_type;
} gdt_structured_t;

void gdt_load(gdt_t* gdt, uint32_t size);
int gdt_structured_to_gdt(gdt_t* gdt, gdt_structured_t* gdt_s, uint32_t total_entries);
#endif //FIRST_ASM_GDT_H
