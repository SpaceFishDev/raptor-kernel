BUILD_DIR?=build/
ASM?=nasm
CC16?=/usr/bin/watcom/binl/wcc
LD16?=/usr/bin/watcom/binl/wlink
CFLAGS16?=-4 -d3 -s -wx -ms -zl -zq -za99
ASMFLAGS= -f obj
SOURCES_C=$(wildcard *.c) disk/disk.c disk/fat.c graphics/video.c keyboard/keyboard.c
SOURCES_ASM=$(wildcard *.asm)
OBJECTS_C=$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.obj, $(SOURCES_C))
OBJECTS_ASM=$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.obj, $(SOURCES_ASM))

.PHONY: all clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin 

$(BUILD_DIR)/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD16) NAME $(BUILD_DIR)/kernel.bin FILE \{ $(OBJECTS_ASM) $(OBJECTS_C)  \} OPTION MAP=$(BUILD_DIR)/kernel.map @linker.lnk		

$(BUILD_DIR)/kernel/c/%.obj: %.c always
	$(CC16) $(CFLAGS16) -fo=$@ $< 

$(BUILD_DIR)/kernel/asm/%.obj: %.asm always
	$(ASM) $(ASMFLAGS) -o $@ $<

always:
	mkdir -p $(BUILD_DIR)/kernel/c
	mkdir -p $(BUILD_DIR)/kernel/asm
	mkdir -p $(BUILD_DIR)/kernel/c/disk/
	mkdir -p $(BUILD_DIR)/kernel/c/graphics/
	mkdir -p $(BUILD_DIR)/kernel/c/keyboard/


clean:
	rm -f $(BUILD_DIR)/kernel.bin
