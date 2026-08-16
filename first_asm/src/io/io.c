//
// Created by steven on 16/8/26.
//
#include <stdint.h>

static const int TERMINAL_WIDTH = 80;
static const int TERMINAL_HEIGHT = 80;
// text mode video memory address: 0xB8000
static uint16_t* video_mem = (uint16_t*) (0xB8000);
static int terminal_row = 0;
static int terminal_col = 0;

static uint16_t make_char(const char c, const char colour) {
    return (colour << 8) | c;
}

static void print_char(const char c) {
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

