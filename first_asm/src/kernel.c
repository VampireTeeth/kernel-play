#include "kernel.h"
#include "io/io.h"
#include "idt/idt.h"

void kernel_main() {
    const char hello[] = "Hello, my kernel!";
    terminal_init();
    print_string(hello);

    idtr_init();
}