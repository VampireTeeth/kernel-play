//
// Created by steven on 25/8/26.
//

#include "string.h"

#include <stdbool.h>

size_t strlen(const char* s)
{
    int c = 0;
    while (*s++ != '\0')
    {
        c++;
    }
    return c;
}

int char_to_numeric(char c)
{
    return (int)c - 48;
}

bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}
