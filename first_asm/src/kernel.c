#include "kernel.h"

#include "config.h"
#include "terminal/terminal.h"
#include "memory/paging/paging.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/pparser.h"
#include "gdt/gdt.h"
#include "memory/memory.h"
#include "string/string.h"
#include "task/tss.h"

tss_t tss;
gdt_t gdt_table[KERNEL_TOTAL_GDT_SEGMENTS];
gdt_structured_t gdt_structured_table[KERNEL_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .access_type = 0x00}, // null segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .access_type = 0x9A}, // kernel code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .access_type = 0x92}, // kernel data segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .access_type = 0xF8}, // user code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .access_type = 0xF2}, // user data segment
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .access_type = 0xE9}, // TSS segment
};
static void demo_pparser();
static void demo_disk_streamer();
static void demo_fopen();
static void demo_fread();
static void demo_fseek();

static void panic(const char* msg)
{
    print_string("\n");
    print_string("panic: ");
    print_string(msg);
    print_string("\n");
    while (1) {}
}

void tss_init()
{
    memset(&tss, 0x00, sizeof(tss_t));
    tss.esp0 = 0x600000; // Kernel stack pointer
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(5 * sizeof(gdt_t)); // tss is the 6th entry, this is the offset from gdt_table
}

void gdt_table_init()
{
    memset(gdt_table, 0, sizeof(gdt_table));
    int total_entries = KERNEL_TOTAL_GDT_SEGMENTS;
    gdt_structured_to_gdt(gdt_table, gdt_structured_table, total_entries);
    gdt_load(gdt_table, total_entries * sizeof(gdt_t) - 1);
}

void kernel_main() {
    int res = 0;
    terminal_init();
    print_string("Welcome!\n");
    idtr_init();
    gdt_table_init();
    res = kheap_init();
    if (res < 0)
    {
        print_string("Failed to create kernel heap\n");
    }
    fs_init();
    disk_search_and_init();
    tss_init();

    uint8_t flags = PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT;
    paging_4gb_chunk* chunk = paging_new_4gb(flags);
    paging_directory_entry_t* directory = paging_4gb_chunk_get_directory(chunk);
    paging_switch(directory);
    paging_enable_paging();

    demo_pparser();
    demo_disk_streamer();
    demo_fopen();
    demo_fread();
    demo_fseek();
    panic("Testing panic");
}

static void count_and_print(const char * const S)
{
    const char* p = S;
    int c = 0;
    while (*p)
    {
        p++;
        c++;
    }
    char s[100];
    itoa(c, s);
    print_string("Total bytes read from file: ");
    print_string(s);
    print_string("\n");
}

static void demo_fopen()
{
    const char* file = "0:/aaa/bbb/hello.txt";
    int fd = fopen(file, "r");
    if (fd)
    {
        print_string("found file:");
        print_string(file);
        print_string("\n");
        fclose(fd);
    }

    file = "0:/aaa/bbb/bighello.txt";
    fd = fopen(file, "r");
    if (fd)
    {
        print_string("found file:");
        print_string(file);
        print_string("\n");
        fclose(fd);
    }
}

static void demo_fread()
{
    const char* file = "0:/aaa/bbb/bighello.txt";
    int fd = fopen(file, "r");
    size_t len = 1024 * 1024 * 2;
    char* out = kheap_zalloc((len+1) * sizeof(char));
    int res = 0;
    res = fread(out, 1024, 1024, fd);
    if (res < 0)
    {
        print_string("Failed to read file!\n");
        goto out;
    }
    out[len] = '\0';
    count_and_print(out);

    out:
    if (out)
    {
        kheap_free(out);
    }
    if (fd)
    {
        fclose(fd);
    }
}

static void demo_fseek()
{
    const char* file = "0:/aaa/bbb/bighello.txt";
    int fd = fopen(file, "r");
    size_t len = 100;
    char* out = kheap_zalloc((len+1) * sizeof(char));
    int res = 0;
    res = fseek(fd, 10, SEEK_SET);
    res = fread(out, 100, 1, fd);
    if (res < 0)
    {
        print_string("Failed to read file!\n");
        goto out;
    }
    out[len] = '\0';
    count_and_print(out);
    print_string(out);
    out:
    if (out)
    {
        kheap_free(out);
    }
    if (fd)
    {
        fclose(fd);
    }
}

void demo_pparser()
{
    const char* path_str = "0:/test/ok/me.txt";
    const char* path_str1 = "1:/another/dir/you.txt";
    path_root_t* path_root = kheap_zalloc(sizeof(path_root_t));
    int r = pparser_parse_path_root(path_str, path_root);
    if (r < 0)
    {
        print_string("Failed to parse path root!\n");
    }
    pparser_free_path_root(path_root);
    path_root_t* path_root1 = kheap_zalloc(sizeof(path_root_t));
    r = pparser_parse_path_root(path_str1, path_root1);
    if (r < 0)
    {
        print_string("Failed to parse path root!\n");
    }
    pparser_free_path_root(path_root1);
}

void demo_disk_streamer()
{
    disk_streamer_t* streamer = disk_streamer_new(0);
    disk_streamer_seek_pos(streamer, 0x201);
    char out[1024];
    disk_streamer_read_bytes(streamer, 1, &out);
    disk_streamer_close(streamer);
}

