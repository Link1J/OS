#include "Terminal.hpp"
#include "CMOS.hpp"
#include "Screen.hpp"
#include "Error.hpp"
#include "MemoryManager.hpp"
#include "GDT.hpp"
#include "IDT.hpp"
#include "VFS.hpp"
#include "ACPI/ACPI.hpp"
#include "PS2Keyboard/PS2Keyboard.hpp"
#include "PIC/PIC.hpp"
#include "TTY/TTY.hpp"
#include "TTY/stdio.hpp"
#include "PCI/PCI.hpp"

#include "KernelHeader.h"
#include "printf.h"

#define TIMEZONE -4

#define KernelMain _start

extern "C" [[noreturn]] void KernelMain(KernelHeader* info)
{
	MemoryManager	::Init(info->physMapStart, info->pageBuffer, info->highMemoryBase	);
	GDT				::Init((uint64_t)info->kernelImage.buffer							);
	IDT				::Init((uint64_t)info->kernelImage.buffer							);
	VFS				::Init(																);

	CMOS::RTC::UpdateBlocking();
	Screen  ::Init(info->screenBuffer, info->screenWidth, info->screenHeight, info->screenColorsInverted);
	Screen  ::Clear({0x00,0x00,0x00,0xFF});

	if (!VFS::CreateFolder("/System", "tty"))
		printf("Failed to create /System/tty\n");
	if (!VFS::CreateFolder("/System", "input"))
		printf("Failed to create /System/input\n");
	if (!VFS::CreateFolder("/System", "IDE"))
		printf("Failed to create /System/IDE\n");
	new TTYScreen({{0xBA,0xDA,0x55,0xFF}}, {{0x00,0x00,0x00,0xFF}});
	new PS2Keyboard();
	stdio::Init();

	PCI::Init();
	PIC::Init();
	PIC::EnableKeyboardIRQ();

	printf("Kernel's Position in memory: %016llX\n", info->kernelImage.buffer);

	auto file = VFS::OpenFile("/System/stdio");
	if (file != 0)
	{
		char help[] = {0,0};
		VFS::WriteFile(file, help, 2);
		VFS::CloseFile(file);
	}

	auto hour = CMOS::RTC::HourTimezone(TIMEZONE);
	
	printf("Startup Time: %2d:%02d:%02d%s %02d/%02d/%4d\n",
			(hour % 12) != 0 ? hour % 12 : 12,
			CMOS::RTC::Minute(), CMOS::RTC::Second(), 
			(hour < 12) ? "AM" : "PM",
			CMOS::RTC::DayTimezone(TIMEZONE), 
			CMOS::RTC::MonthTimezone(TIMEZONE),
			CMOS::RTC::YearTimezone(TIMEZONE)
			);

	printf("Screen Info: %d, %d\n", Screen::Width(), Screen::Height());

	asm("sti");

	Terminal::Run();
	
	Error::Panic("Reached end of main");
	for (;;);
}