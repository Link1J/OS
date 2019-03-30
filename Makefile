ARCH			= x86_64
BOOTLOADER		= build/boot-$(ARCH).efi
KERNEL			= build/kernel-$(ARCH).elf

HDD_IMAGE		= image.img

COMMON_SRCS_C	= $(wildcard src/common/*.c)
COMMON_SRCS_CPP	= $(wildcard src/common/*.cpp)
COMMON_INCS_H	= -I src/common

C++_SRCS_CPP	= $(wildcard src/c++/*.cpp)
C++_OBJS_COP	= $(patsubst src/%,build/%,$(patsubst %.cpp,%.o,$(C++_SRCS_CPP)))
C++_OBJS		= $(C++_OBJS_COP)

ACPICA_SRCS_C	= $(wildcard src/drivers/ACPI/ACPICA/components/**/*.c)
ACPICA_OBJS		= $(patsubst src/%,build/%,$(patsubst %.c,%.c.o,$(ACPICA_SRCS_C)))

KERNEL_INCS_HPP	= -I src/system -I src/core -I src/memory -I src/filesystem -I src/drivers
KERNEL_SRCS_CPP	= $(wildcard src/core/*.cpp) $(wildcard src/memory/*.cpp) $(wildcard src/system/*.cpp) $(wildcard src/filesystem/*.cpp) $(wildcard src/drivers/**/*.cpp)
KERNEL_SRCS_ASM	= $(wildcard src/core/*.asm)
KERNEL_OBJS_SRC	= $(patsubst src/%,build/%,$(patsubst %.c,%.c.o,$(KERNEL_SRCS_C)))
KERNEL_OBJS_SRP	= $(patsubst src/%,build/%,$(patsubst %.cpp,%.cpp.o,$(KERNEL_SRCS_CPP)))
KERNEL_OBJS_COC	= $(patsubst src/%,build/%,$(patsubst %.c,%.c.o,$(COMMON_SRCS_C)))
KERNEL_OBJS_COP	= $(patsubst src/%,build/%,$(patsubst %.cpp,%.cpp.o,$(COMMON_SRCS_CPP)))
KERNEL_OBJS_ASM = $(patsubst src/%,build/%,$(patsubst %.asm,%.asm.o,$(KERNEL_SRCS_ASM)))
KERNEL_OBJS		= $(KERNEL_OBJS_COC) $(KERNEL_OBJS_SRP) $(KERNEL_OBJS_SRC) $(KERNEL_OBJS_ASM) $(KERNEL_OBJS_COP)

BOOT_SRCS_C		= $(wildcard src/boot/*.cpp)
BOOT_OBJS		= $(patsubst src/%,build/%, $(patsubst %.cpp,%.o,$(BOOT_SRCS_C)))

EFI_INC			= /usr/include/efi
EFI_INCS		= -I$(EFI_INC) -I$(EFI_INC)/$(ARCH) -I$(EFI_INC)/protocol

BOOT_C_FLAGS	= $(EFI_INCS) $(COMMON_INCS_H) -ffreestanding
BOOT_LD_FLAGS	= -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main 

KERNEL_C_FLAGS	= $(COMMON_INCS_H) $(KERNEL_INCS_HPP) -g -fPIC -ffreestanding -fno-exceptions -mno-red-zone -fno-rtti
KERNEL_AS_FLAGS	= $(subst -I,-i,$(COMMON_INCS_H) $(KERNEL_INCS_HPP))
KERNEL_LD_FLAGS	= -g -fPIC -nostdlib -shared -mfloat-abi=soft

CC_KERN = clang
CC_BOOT = clang -target x86_64-w64-mingw32

QEMU = qemu-system-$(ARCH)

WSLENV ?= notwsl
ifndef WSLENV
QEMU = qemu-system-$(ARCH).exe
endif

.PHONY: clean build run buildHDImg createHDImg disassemble

build: $(BOOTLOADER) $(KERNEL)

all: build disassemble buildHDImg run

createHDImg: $(HDD_IMAGE)

$(HDD_IMAGE):
	rm -rf $(HDD_IMAGE)
	dd if=/dev/zero of=$(HDD_IMAGE) bs=512 count=205048
	parted $(HDD_IMAGE) -s -a minimal mklabel gpt
	parted $(HDD_IMAGE) -s -a minimal mkpart EFI FAT32 2048s 204800s
	parted $(HDD_IMAGE) -s -a minimal toggle 1 boot
	
buildHDImg: $(BOOTLOADER) $(KERNEL) $(HDD_IMAGE)
	dd if=/dev/zero of=build/part.img bs=512 count=204800
	mkfs.fat build/part.img
	mmd -i build/part.img ::/EFI
	mmd -i build/part.img ::/EFI/BOOT
	
	cp $(BOOTLOADER) build/bootx64.efi
	cp $(KERNEL) build/kernel.elf
	strip build/kernel.elf
	
	mcopy -i build/part.img build/bootx64.efi ::/EFI/BOOT
	mcopy -i build/part.img build/kernel.elf ::/
	
	dd if=build/part.img of=$(HDD_IMAGE) bs=512 count=204800 seek=2048 conv=notrunc
	rm -rf build/part.img build/bootx64.efi build/kernel.elf

clean:
	rm -rf build
	
disassemble:
	rm -rf dis.txt
	objdump -S -d -C -g -M intel-mnemonic --no-show-raw-insn build/kernel-x86_64.elf >> dis.txt

run:
	rm -rf log.txt
	$(QEMU) -bios OVMF.fd -drive file=$(HDD_IMAGE),media=disk,format=raw -m 2048M -s -d cpu_reset -D log.txt -serial file:serial.log
	#-S

$(BOOTLOADER): $(BOOT_OBJS)
	mkdir -p $(dir $@)
	$(CC_BOOT) $(BOOT_LD_FLAGS) -o $@ $^
	
$(KERNEL) : $(KERNEL_OBJS) $(C++_OBJS) $(ACPICA_OBJS)
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_LD_FLAGS) -o $(KERNEL) $(C++_OBJS_CRTI) $(C++_OBJS_COP) $(KERNEL_OBJS) $(ACPICA_OBJS) $(C++_OBJS_CRTN) 
	
build/boot/%.o: src/boot/%.cpp
	mkdir -p $(dir $@)
	$(CC_BOOT)  $(BOOT_C_FLAGS) -c $< -o $@

build/boot/%.o: src/common/%.c
	mkdir -p $(dir $@)
	$(CC_BOOT) $(BOOT_C_FLAGS) -c $< -o $@

build/common/%.cpp.o: src/common/%.cpp
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@
	
build/common/%.c.o: src/common/%.c
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@
	
build/core/%.cpp.o: src/core/%.cpp
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@
	
build/memory/%.cpp.o: src/memory/%.cpp
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@
	
build/system/%.cpp.o: src/system/%.cpp
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@

build/c++/%.o: src/c++/%.asm
	mkdir -p $(dir $@)
	as -g --64 $< -o $@
	
build/c++/%.o: src/c++/%.cpp
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@

build/core/%.asm.o: src/core/%.asm
	mkdir -p $(dir $@)
	nasm -g -f elf64 $(KERNEL_AS_FLAGS) $< -o $@

build/filesystem/%.cpp.o: src/filesystem/%.cpp
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@

build/drivers/%.cpp.o: src/drivers/%.cpp
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@

build/drivers/ACPI/ACPICA/components/%.c.o: src/drivers/ACPI/ACPICA/components/%.c
	mkdir -p $(dir $@)
	$(CC_KERN) $(KERNEL_C_FLAGS) -c $< -o $@ -include src/drivers/ACPI/acenv.h -include src/drivers/ACPI/acenvex.h -isystem src/drivers/ACPI/ACPICA/include