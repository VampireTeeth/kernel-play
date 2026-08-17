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
extern void enable_interrupt();

// handles IRQ 1 (keyboard pressed IRQ), PIC vector offset is remapped to 0x20
// so the IRQ1 interrupt number is 0x21
extern void int21h();
extern void no_interrupt();

void int21h_handler()
{
    print_string("Keyboard pressed!!!");
    outb(0x20, 0x20); // ack to master PIC command port 0x20
}

void no_interrupt_handler()
{
    outb(0x20, 0x20); // ack to master PIC command port 0x20
}

typedef struct interrupt_frame interrupt_frame;

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
    for (int i = 0; i < OS_TOTAL_INTERRUPTS; i++)
    {
        set_interrupt_handler(i, no_interrupt);
    }
    set_interrupt_handler(0x21, int21h);
    idt_load(&idtr_descriptor);
    enable_interrupt();
}
