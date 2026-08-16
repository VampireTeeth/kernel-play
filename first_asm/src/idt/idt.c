//
// Created by steven on 16/8/26.
//
#include <stdint.h>
#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "io/io.h"

static idt_desc idt_descriptors[OS_TOTAL_INTERRUPTS];
static idtr_desc idtr_descriptor;

extern void idt_load(idtr_desc* ptr);

typedef struct interrupt_frame interrupt_frame;

__attribute__((interrupt)) void is_zero(interrupt_frame * frame)
{
     print_string("Divided by zero!!!");
}

static void set_interrupt_handler(int int_no, void* handler_addr)
{
    idt_desc* idt_p = &idt_descriptors[int_no];
    idt_p->offset_1 = (uint32_t) handler_addr & 0xFFFF;
    idt_p->selector = KERNEL_CODE_SELECTOR;
    idt_p->type_attributes = 0xEE; // b11101110
    idt_p->zero = 0x00;
    idt_p->offset_2 = (uint32_t) handler_addr >> 16;
}

void idtr_init()
{
    memset(&idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)&idt_descriptors;
    set_interrupt_handler(0, is_zero);
    idt_load(&idtr_descriptor);
}
