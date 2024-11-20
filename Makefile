CC=gcc
CFLAGS=-Wall -Wextra
KERNEL_DIR=/lib/modules/$(shell uname -r)/build

# Kernel module
obj-m += 0.o

all: kernel userspace receiver

kernel:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

userspace: 0
	@if [ ! -f src/0.c ]; then \
		echo "Error: src/0.c not found"; \
		exit 1; \
	fi
	@if [ ! -f src/read_event.asm ]; then \
		echo "Error: src/read_event.asm not found"; \
		exit 1; \
	fi
	@command -v nasm >/dev/null 2>&1 || { echo "Error: nasm is not installed"; exit 1; }

receiver: src/receiver.c
	$(CC) $(CFLAGS) -I./include -o receiver src/receiver.c

0: src/0.c src/read_event.asm
	nasm -f elf64 src/read_event.asm -o read_event.o
	$(CC) $(CFLAGS) -o 0 src/0.c read_event.o -I./include

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f 0 receiver read_event.o
	rm -f *.o *.ko *.mod.* Module.* modules.*

.PHONY: all clean kernel userspace receiver
