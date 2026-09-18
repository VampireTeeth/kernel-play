#!/bin/bash
export PREFIX=$HOME/opt/cross
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
make clean
make all
make cp_file
make qemu_debug