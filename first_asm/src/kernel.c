#include <stdint.h>
#include "kernel.h"

const int TERMINAL_WIDTH = 80;
const int TERMINAL_HEIGHT = 80;
// text mode video memory address: 0xB8000
uint16_t* video_mem = (uint16_t*) (0xB8000);
int terminal_row = 0;
int terminal_col = 0;

uint16_t make_char(char c, char colour) {
    return (colour << 8) | c;
}

void print_char(char c) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        return;
    }
    video_mem[terminal_row * TERMINAL_WIDTH + terminal_col] = make_char(c, 0xF);
    terminal_col++;
    if (terminal_col >= TERMINAL_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }
}

void print_string(const char* input) {
    for(const char* c = input; *c != '\0'; c++) {
        print_char(*c);
    }
}

void terminal_init() {
    for (int i = 0; i < TERMINAL_WIDTH; i++) {
        for (int j = 0; j < TERMINAL_HEIGHT; j++) {
            video_mem[j*TERMINAL_WIDTH + i] = make_char(' ', 0);
        }
    }
}

void kernel_main() {
    terminal_init();
    char hello[] = "Hello, my kernel!";
    print_string(hello);
    return;
}