TARGET = target
BOOT_SRC_FILE = src/boot/boot.asm
KERNEL_SRC_FILE = src/kernel.asm
KERNEL_C_SRC_FILES = src/kernel.c
LINKER_LD = ./linker.ld

# Derive Basename (e.g., "hello" from "src/hello.asm")
# $(notdir ...) removes the directory path -> "hello.asm"
# $(basename ...) removes the extension -> "hello"
BOOT_BASE := $(basename $(notdir $(BOOT_SRC_FILE)))

# Construct Output Paths Automatically
BOOT_BIN = $(TARGET)/$(BOOT_BASE).bin

KERNEL_OBJ = $(TARGET)/kernel.asm.o
KERNEL_C_OBJ = $(TARGET)/kernel.o
KERNEL_FULL_OBJ = $(TARGET)/kernelfull.o

KERNEL_BIN = $(TARGET)/kernel.bin

OBJS = $(KERNEL_OBJ) $(KERNEL_C_OBJ) # The kernel.asm.o needs to be the first obj entry
OS_BIN = $(TARGET)/os.bin
OS_IMG = $(TARGET)/os.img

BOCHS_BIN=bochs

INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

$(TARGET):
	mkdir -p $(TARGET)

$(BOOT_BIN):  $(BOOT_SRC_FILE) | $(TARGET)
	nasm -f bin $(BOOT_SRC_FILE) -o $(BOOT_BIN)

$(KERNEL_OBJ): $(KERNEL_SRC_FILE) | $(TARGET)
	nasm -f elf -g $(KERNEL_SRC_FILE) -o $(KERNEL_OBJ)

$(KERNEL_C_OBJ): $(KERNEL_C_SRC_FILES) | $(TARGET)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c $(KERNEL_C_SRC_FILES) -o $(KERNEL_C_OBJ)

$(KERNEL_BIN): $(OBJS)
	i686-elf-ld -g -relocatable $(OBJS) -o $(KERNEL_FULL_OBJ)
	i686-elf-gcc $(FLAGS) -T $(LINKER_LD) -o $(KERNEL_BIN) $(KERNEL_FULL_OBJ)

$(OS_BIN): $(BOOT_BIN) $(KERNEL_BIN)
	dd if=$(BOOT_BIN) of=$(OS_BIN) status=none
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	dd if=/dev/zero bs=512 count=1000 >> $(OS_BIN)

kernel: $(OS_BIN)
	echo "Build Kernel"

run_bochs: $(OS_BIN)
	dd if=/dev/zero of=$(OS_IMG) bs=512 count=2880 status=none
	dd if=$(OS_BIN) of=$(OS_IMG) conv=notrunc status=none
	$(BOCHS_BIN) -qf /dev/null \
		"floppya: 1_44=$(OS_IMG), status=inserted" \
		"boot: floppy" \
		"display_library: sdl2" \
		"megs: 128"

qemu_debug: $(OS_BIN)
	gdb \
		--eval-command="set confirm off" \
		--eval-command="target remote | qemu-system-x86_64 -drive format=raw,file=$(OS_BIN) -display gtk -S -gdb stdio" \
		--eval-command="add-symbol-file $(KERNEL_FULL_OBJ) 0x00100000" \
		--eval-command="break kernel_main"

qemu_run: $(OS_BIN)
	qemu-system-x86_64 -drive format=raw,file=$(OS_BIN) -display gtk

clean:
	rm -f $(OBJS) $(BOOT_BIN) $(KERNEL_BIN) $(OS_BIN) $(OS_IMG) $(KERNEL_FULL_OBJ)