//
// Created by steven on 16/8/26.
//
#ifndef IDT_H
#define IDT_H
#include <stdint.h>
typedef struct idt_desc
{
    uint16_t offset_1;        // offset bits 0..15
    uint16_t selector;        // a code segment selector in GDT or LDT
    uint8_t  zero;            // unused, set to 0
    uint8_t  type_attributes; // gate type, dpl, and p fields
    uint16_t offset_2;        // offset bits 16..31
} __attribute__((packed)) idt_desc;

typedef struct idtr_desc
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_desc;

void idtr_init();
#endif // IDT_H
