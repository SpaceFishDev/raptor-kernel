ASM=nasm
CC16=/usr/bin/watcom/binl/wcc
LD16=/usr/bin/watcom/binl/wlink

SRC=src
BUILD=build


.PHONY: all floppy_img kernel bootloader clean always

floppy_img: $(BUILD)/os.img

$(BUILD)/os.img:  kern
		dd if=/dev/zero of=$(BUILD)/os.img bs=512 count=2880
		mkfs.fat -F 12 -n "NBOS" $(BUILD)/os.img
		dd if=$(BUILD)/stage1.bin of=$(BUILD)/os.img conv=notrunc
		mcopy -i $(BUILD)/os.img $(BUILD)/kernel.bin "::kernel.bin"
		mcopy -i $(BUILD)/os.img test.txt "::test.txt"
		mcopy -o test  -i build/os.img "::dir"
		qemu-system-i386 -fda $(BUILD)/os.img
kern: stage1 kernel 
stage1: $(BUILD)/stage1.bin
$(BUILD)/stage1.bin: always
	$(MAKE) -C $(SRC)/bootloader/stage1/ BUILD_DIR=$(abspath $(BUILD))

kernel: $(BUILD)/kernel.bin
$(BUILD)/kernel.bin: always
	$(MAKE) -C $(SRC)/kernel/ BUILD_DIR=$(abspath $(BUILD))
always:
	mkdir -p $(BUILD)

clean:
	find ./ -type f \( -name "*.bin" -o -name "*.o" -o -name "*.err" \) -delete


run:
	qemu-system-i386 -fda $(BUILD)/os.img
