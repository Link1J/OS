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
#include "stdio.hpp"

#include "KernelHeader.h"
#include "printf.h"

#define TIMEZONE -4

#define KernelMain _start

extern "C" [[noreturn]] void KernelMain(KernelHeader* info)
{
	CMOS::RTC::UpdateBlocking();
	Screen  ::Init(info->screenBuffer, info->screenWidth, info->screenHeight, info->screenColorsInverted);
	Screen  ::Clear({0x00,0x00,0x00,0xFF});

	MemoryManager	::Init(info->physMapStart, info->pageBuffer, info->highMemoryBase	);
	GDT				::Init((uint64_t)info->kernelImage.buffer							);
	IDT				::Init((uint64_t)info->kernelImage.buffer							);
	VFS				::Init(																);

	new TTYScreen({{0xBA,0xDA,0x55,0xFF}}, {{0x00,0x00,0x00,0xFF}});
	new PS2Keyboard();

	stdio::Init();	
	
	printf("Kernel's Position in memory: %016llX\n", info->kernelImage.buffer);
	printf("Screen Info: %d, %d\n", Screen::Width(), Screen::Height());
	printf("Terminal Info: %d, %d\n", Terminal::CharWidth(), Terminal::CharHeight());
	
	auto hour = CMOS::RTC::HourTimezone(TIMEZONE);
	
	printf("Startup Time: %2d:%02d:%02d%s %02d/%02d/%4d\n",
			(hour % 12) != 0 ? hour : 12,
			CMOS::RTC::Minute(), CMOS::RTC::Second(), 
			(hour < 12) ? "AM" : "PM",
			CMOS::RTC::DayTimezone(TIMEZONE), 
			CMOS::RTC::MonthTimezone(TIMEZONE),
			CMOS::RTC::YearTimezone(TIMEZONE)
			);
	
	
	PIC::Init();
	PIC::EnableKeyboardIRQ();
	
	auto file = VFS::OpenFile("/Devices/tty/stdio");

	asm("sti");

	Terminal::Run();

	//ACPI::Init(info->RSDPStructure	);

	//Error::Panic("Reached end of main");
	for(;;) {
		asm("hlt");
	}
}