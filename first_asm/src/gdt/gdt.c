//
// Created by steven on 22/9/26.
//
#include "gdt.h"

#include "config.h"

int gdt_structured_to_gdt(gdt_t* gdt, gdt_structured_t* gdt_s, uint32_t total_entries)
{
    for (int i = 0; i < total_entries; i++)
    {
        if (gdt_s->limit > 0xFFFF && (gdt_s->limit & 0xFFF) != 0xFFF)
        {
            return -EINVARG;
        }
        uint32_t limit = gdt_s->limit;

        gdt->flags_and_limit = 0x40; //0b01000000 G is set to 0, indicating limit is in 1B unit
        if (limit > 0xFFFF)
        {
            // Automatically turn into page granularity when limit > 0xFFFF (16*4KB, which is 64KB)
            limit = limit >> 12; // the limit in gdt_structured is always in 1B unit, so converting to 4KB unit here for limit > 4KB
            gdt->flags_and_limit = 0xC0; //0x11000000 G is set to 1, indicating limit is in 4KB unit
        }
        // limit
        gdt->limit_first = limit & 0xFFFF; // take the lower 16-bit of limit
        // flags 4-bits: G|DB|L|Reserved
        gdt->flags_and_limit |= (limit >> 16) & 0x0F; // take the higher 4-bit of limit

        // base
        gdt->base_first = gdt_s->base & 0xFFFF;
        gdt->base = (gdt_s->base >> 16) & 0xFF;
        gdt->base_24_31_bits = (gdt_s->base >> 24) & 0xFF;

        // type
        gdt->access_type = gdt_s->access_type;
        gdt_s++;
        gdt++;
    }
    return 0;
}
