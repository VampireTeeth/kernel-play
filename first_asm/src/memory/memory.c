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