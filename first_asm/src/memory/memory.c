//
// Created by steven on 16/8/26.
//

#include "memory.h"

void* memset(void* ptr, const int c, const size_t size)
{
    char* p = ptr;
    for (int i = 0; i < size; i++)
    {
        p[i] = (char)c;
    }
    return ptr;
}

int memcmp(const void* lhs, const void* rhs, const size_t size)
{
    const char* l = lhs;
    const char* r = rhs;
    for (int i = 0; i < size; i++)
    {
        if (l[i] != r[i])
        {
            return l[i] < r[i] ? -1 : 1;
        }
    }
    return 0;
}

void memcpy(void* dest, const void* src, const size_t size)
{
    char* d = dest;
    const char* s = src;
    for (int i = 0; i < size; i++)
    {
        d[i] = s[i];
    }
}