//
// Created by steven on 16/8/26.
//

#ifndef FIRST_ASM_IO_H
#define FIRST_ASM_IO_H

// termimal print functions
void print_string(const char* str);

void terminal_init();


// IN and  out functions
unsigned char insb(unsigned short port);
unsigned short insw(unsigned short port);
void outb(unsigned short port, unsigned char data);
void outw(unsigned short port, unsigned short data);

#endif //FIRST_ASM_IO_H
