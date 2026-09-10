//
// Created by steven on 25/8/26.
//

#ifndef FIRST_ASM_STRING_H
#define FIRST_ASM_STRING_H
#include <stdbool.h>
#include <stddef.h>

size_t strlen(const char* s);

size_t strlen_terminator(const char* s, char terminator);

char* strcpy(char* dst, char* src);

int strncmp(const char* lhs, const char* rhs, int len);

int istrncmp(const char* lhs, const char* rhs, int len);

int char_to_numeric(char c);

char tolower(char c);

bool is_digit(char c);
#endif //FIRST_ASM_STRING_H
