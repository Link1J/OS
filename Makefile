ARCH			= x86_64
BOOTLOADER		= build/boot-$(ARCH).efi
KERNEL			= build/kernel-$(ARCH).elf

HDD_IMAGE		= image.img

COMMON_SRCS_C	= $(wildcard src/common/*.c)
COMMON_INCS_H	= -I src/common

KERNEL_SRCS_CPP	= $(wildcard src/core/*.cpp) $(wildcard src/mm/*.cpp)
KERNEL_OBJS		= $(patsubst src/%,build/%, $(patsubst %.cpp,%.o,$(KERNEL_SRCS_CPP)) $(patsubst %.c,%.o,$(COMMON_SRCS_C)))

BOOT_SRCS_C		= $(wildcard src/boot/*.c)
BOOT_OBJS		= $(patsubst src/%,build/%, $(patsubst %.c,%.o,$(BOOT_SRCS_C))) $(patsubst src/common/%,build/boot/%,$(patsubst %.c,%.o,$(COMMON_SRCS_C)))

EFI_INC			= /usr/include/efi
EFI_INCS		= -I$(EFI_INC) -I$(EFI_INC)/$(ARCH) -I$(EFI_INC)/protocol

BOOT_C_FLAGS	= $(EFI_INCS) $(COMMON_INCS_H) -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -Wall -Wno-incompatible-library-redeclaration -O2 -static
BOOT_LD_FLAGS	= -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -lgcc

KERNEL_C_FLAGS	= $(COMMON_INCS_H) -fno-stack-protector -fpic -mno-red-zone -Wall -ffreestanding
KERNEL_LD_FLAGS	= -nostdlib -e kernel_main -fPIC  -static -lgcc

.PHONY: clean build run buildHDImg createHDImg

build: $(BOOTLOADER) $(KERNEL)

all: clean build buildHDImg run

createHDImg:
	rm -rf $(HDD_IMAGE)
	dd if=/dev/zero of=$(HDD_IMAGE) bs=512 count=104448
	parted $(HDD_IMAGE) -s -a minimal mklabel gpt
	parted $(HDD_IMAGE) -s -a minimal mkpart EFI FAT32 2048s 102400s
	parted $(HDD_IMAGE) -s -a minimal toggle 1 boot
	
buildHDImg: build
	dd if=/dev/zero of=build/part.img bs=512 count=102400
	mkfs.fat build/part.img
	mmd -i build/part.img ::/EFI
	mmd -i build/part.img ::/EFI/BOOT
	
	cp $(BOOTLOADER) build/bootx64.efi
	cp $(KERNEL) build/kernel.elf
	mcopy -i build/part.img build/bootx64.efi ::/EFI/BOOT
	mcopy -i build/part.img build/kernel.elf ::/
	
	dd if=build/part.img of=$(HDD_IMAGE) bs=512 count=102400 seek=2048 conv=notrunc
	rm -rf build/part.img build/bootx64.efi build/kernel.elf

clean:
	rm -rf build

run:
	qemu-system-$(ARCH).exe -cpu qemu64 -bios OVMF.fd -drive file=$(HDD_IMAGE),media=disk

$(BOOTLOADER): $(BOOT_OBJS)
	mkdir -p $(dir $@)
	x86_64-w64-mingw32-gcc  $(BOOT_LD_FLAGS) $(BOOT_OBJS) -o $(BOOTLOADER)
	
$(KERNEL) : $(KERNEL_OBJS)
	mkdir -p $(dir $@)
	gcc $(KERNEL_OBJS) $(KERNEL_LD_FLAGS) -o $(KERNEL)
	
build/boot/%.o: src/boot/%.c
	mkdir -p $(dir $@)
	x86_64-w64-mingw32-gcc $(BOOT_C_FLAGS) -c $< -o $@

build/boot/%.o: src/common/%.c
	mkdir -p $(dir $@)
	x86_64-w64-mingw32-gcc $(BOOT_C_FLAGS) -c $< -o $@
	
build/common/%.o: src/common/%.c
	mkdir -p $(dir $@)
	x86_64-w64-mingw32-gcc $(KERNEL_C_FLAGS) -c $< -o $@
	
build/core/%.o: src/core/%.cpp
	mkdir -p $(dir $@)
	clang $(KERNEL_C_FLAGS) -c $< -o $@
	
build/mm/%.o: src/mm/%.cpp
	mkdir -p $(dir $@)
	clang $(KERNEL_C_FLAGS) -c $< -o $@