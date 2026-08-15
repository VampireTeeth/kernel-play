#include "kernel.h"

void kernel_main()
{
    // text mode video address: 0xB8000
     char* video_mem = (char*) (0xB8000);
    video_mem[0] = 'A';
    video_mem[1] = 5;
}