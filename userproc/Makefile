CC      = gcc
LD      = ld
CFLAGS  = -m32 -static -ggdb -MD -Wall -Werror -I./include -O2 \
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer
ASFLAGS = -m32 -MD -I./include
LDFLAGS = -melf_i386
QEMU    = qemu-system-i386

CFILES  = $(shell find . -name "*.c")
SFILES  = $(shell find . -name "*.S")
OBJS    = $(CFILES:.c=.o) $(SFILES:.S=.o)

run: disk.img
	$(QEMU) -serial stdio disk.img

debug: disk.img
	$(QEMU) -serial stdio -s -S disk.img

testproc: $(OBJS)
	$(LD) $(LDFLAGS) -e main -o testproc $(OBJS)
	objdump -D testproc > code.txt	# disassemble result
	readelf -a testproc > elf.txt		# obtain more information about the executable

-include $(OBJS:.o=.d)

clean:
	@cd boot; make clean
	rm -f testproc disk.img $(OBJS) $(OBJS:.o=.d)

NLINES = $(shell find -iname "*.[chS]" | xargs grep -v "^$$" | wc -l)

count:
	@echo "$(NLINES) lines in total."
	
