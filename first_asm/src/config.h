//
// Created by steven on 16/8/26.
//

#ifndef CONFIG_H
#define _CONFIG_H

#define KERNEL_CODE_SELECTOR 0x8
#define KERNEL_DATA_SELECTOR 0x10
#define OS_TOTAL_INTERRUPTS 512
#define DISK_SECTOR_SIZE 512

#define KERNEL_HEAP_SIZE_IN_BYTES 0x6400000 // 100MB
#define KERNEL_HEAP_BLOCK_SIZE 4096

// https://wiki.osdev.org/Memory_Map_(x86)
#define KERNEL_HEAP_ADDRESS 0x01000000
#define KERNEL_HEAP_TABLE_ADDRESS 0x00007E00 // this address is in the real mode, but we dont need bootload at this point


#define OK 0
#define EIO 1
#define EINVARG 2
#define ENOMEM 3

#endif
