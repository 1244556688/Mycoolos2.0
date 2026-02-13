# Makefile
CC = gcc
ASM = nasm
LD = ld

# 編譯參數 (32-bit, freestanding 表示沒有標準函式庫)
CFLAGS = -m32 -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

all: myos.bin

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

boot.o: boot.asm
	$(ASM) $(ASFLAGS) boot.asm -o boot.o

multiboot_header.o: multiboot_header.asm
	$(ASM) $(ASFLAGS) multiboot_header.asm -o multiboot_header.o

myos.bin: multiboot_header.o boot.o kernel.o linker.ld
	$(LD) $(LDFLAGS) -o myos.bin multiboot_header.o boot.o kernel.o

run: myos.bin
	qemu-system-i386 -kernel myos.bin

clean:
	rm -f *.o *.bin
