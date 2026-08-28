TARGET = target
BOOT_SRC_FILE = src/boot/boot.asm
KERNEL_SRC_FILE = src/kernel.asm
KERNEL_C_SRC_FILES = src/kernel.c src/idt/idt.c
LINKER_LD = ./linker.ld

# Derive Basename (e.g., "hello" from "src/hello.asm")
# $(notdir ...) removes the directory path -> "hello.asm"
# $(basename ...) removes the extension -> "hello"
BOOT_BASE := $(basename $(notdir $(BOOT_SRC_FILE)))

# Construct Output Paths Automatically
BOOT_BIN = $(TARGET)/$(BOOT_BASE).bin

KERNEL_OBJ = $(TARGET)/kernel.asm.o
KERNEL_C_OBJ = $(TARGET)/kernel.o $(TARGET)/idt.o
KERNEL_FULL_OBJ = $(TARGET)/kernelfull.o

KERNEL_ELF = $(TARGET)/kernel.elf
KERNEL_BIN = $(TARGET)/kernel.bin

# Find all source files
EXCLUDED_ASM_SRCS := src/add.asm src/hello.asm src/scratch.asm
ASM_SRCS := $(wildcard src/kernel.asm src/idt/*.asm src/io/*.asm src/memory/paging/*.asm)  # The kernel.asm needs to be the first, so the kernel.asm.o will be first in OBJS
C_SRCS := $(wildcard src/*.c src/idt/*.c src/memory/*.c src/io/*.c src/memory/heap/*.c src/memory/paging/*.c src/disk/*.c src/string/*.c src/fs/*.c)
# Transform source paths to object paths (e.g., src/main.c -> target/src/main.o)
OBJS := $(patsubst src/%.asm,$(TARGET)/%.asm.o,$(ASM_SRCS)) $(patsubst src/%.c,$(TARGET)/%.o,$(C_SRCS)) # The kernel.asm.o needs to be the first obj entry
OS_BIN = $(TARGET)/os.bin
OS_IMG = $(TARGET)/os.img

BOCHS_BIN=bochs

INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

$(TARGET):
	mkdir -p $(TARGET)

$(BOOT_BIN):  $(BOOT_SRC_FILE) | $(TARGET)
	nasm -f bin $(BOOT_SRC_FILE) -o $(BOOT_BIN)

$(TARGET)/%.asm.o: src/%.asm | $(TARGET)
	mkdir -p $(dir $@)
	nasm -f elf -F dwarf -g $< -o $@

$(TARGET)/%.o: src/%.c | $(TARGET)
	mkdir -p $(dir $@)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $< -o $@

$(KERNEL_ELF): $(OBJS)
	i686-elf-ld -g -relocatable $(OBJS) -o $(KERNEL_FULL_OBJ)
	i686-elf-gcc $(FLAGS) -T $(LINKER_LD) -o $@ $(KERNEL_FULL_OBJ)

$(KERNEL_BIN): $(KERNEL_ELF)
	i686-elf-objcopy -O binary $< $@

$(OS_BIN): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=$(BOOT_BIN) of=$(OS_BIN) status=none
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	dd if=/dev/zero bs=512 count=1000 >> $(OS_BIN)

all: $(OS_BIN)
	echo "Build Kernel"

run_bochs: $(OS_BIN)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880 status=none
	dd if=$(OS_BIN) of=$(OS_IMG) conv=notrunc status=none
	$(BOCHS_BIN) -qf /dev/null \
		"floppya: 1_44=$(OS_IMG), status=inserted" \
		"boot: floppy" \
		"display_library: sdl2" \
		"megs: 128"

qemu_debug: $(KERNEL_ELF) $(OS_BIN)
	gdb \
		--eval-command="set confirm off" \
		--eval-command="set architecture i386" \
		--eval-command="file $(KERNEL_ELF)" \
		--eval-command="target remote | qemu-system-i386 -drive file=$(OS_BIN),index=0,media=disk -display gtk -S -gdb stdio -no-reboot -d int,cpu_reset -D $(TARGET)/qemu.log" \
		--eval-command="break kernel_main"

qemu_run: $(OS_BIN)
	qemu-system-i386 -drive file=$(OS_BIN),index=0,media=disk -display gtk

clean:
	rm -f $(OBJS) $(BOOT_BIN) $(KERNEL_ELF) $(KERNEL_BIN) $(OS_BIN) $(OS_IMG) $(KERNEL_FULL_OBJ)