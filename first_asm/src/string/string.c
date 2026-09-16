//
// Created by steven on 25/8/26.
//

#include "string.h"

#include <stdbool.h>

size_t strlen(const char* s)
{
    size_t i = 0;
    while (s[i] != '\0') i++;
    return i;
}

size_t strlen_terminator(const char* s, char terminator)
{
    size_t i = 0;
    while (s[i] != '\0' && s[i] != terminator) i++;
    return i;
}

char* strcpy(char* dst, char* src)
{
    char* s = src;
    char* r = dst;
    while (*s!= '\0')
    {
        *r = *s;
        s++;
        r++;
    }
    *r = '\0';
    return r;
}


int strncmp(const char* lhs, const char* rhs, int len)
{
    int r = 0;
    for (int i = 0; i < len; i++)
    {
        if (lhs[i] != rhs[i])
        {
            r = (lhs[i] - rhs[i]) > 0 ? 1 : -1;
            break;
        }
        if (lhs[i] == '\0') break;
    }
    return r;
}

char tolower(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        // only convert the uppercase chars
        return c + 32;
    }
    return c;
}

char toupper(char c)
{
    if (c >= 'a' && c <= 'c')
    {
        // only convert the lowercase chars
        return c - 32;
    }
    return c;
}

int istrncmp(const char* lhs, const char* rhs, int len)
{
    int r = 0;
    for (int i = 0; i < len; i++)
    {
        if (lhs[i] != rhs[i] && tolower(lhs[i]) != tolower(rhs[i]))
        {
            r = (lhs[i] - rhs[i]) > 0 ? 1 : -1;
            break;
        }
        if (lhs[i] == '\0') break;
    }
    return r;
}

int char_to_numeric(char c)
{
    return (int)c - 48;
}

bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}
