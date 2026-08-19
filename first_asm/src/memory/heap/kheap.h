//
// Created by steven on 18/8/26.
//

#ifndef FIRST_ASM_KHEAP_H
#define FIRST_ASM_KHEAP_H
#include <stddef.h>

void kheap_init();
void* kheap_malloc(size_t size);
void kheap_free(void* ptr);
#endif //FIRST_ASM_KHEAP_H
