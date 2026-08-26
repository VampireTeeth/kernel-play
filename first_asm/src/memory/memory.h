//
// Created by steven on 16/8/26.
//

#ifndef FIRST_ASM_MEMORY_H
#define FIRST_ASM_MEMORY_H
#include <stddef.h>

void* memset(void* ptr, int c, size_t size);
int memcmp(const void* lhs, const void* rhs, const size_t size);
void memcpy(void* dest, const void* src, const size_t size);
#endif //FIRST_ASM_MEMORY_H
