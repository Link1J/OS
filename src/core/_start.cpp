#include "Terminal.hpp"
#include "CMOS.hpp"
#include "Screen.hpp"
#include "Error.hpp"
#include "MemoryManager.hpp"
#include "GDT.hpp"
#include "IDT.hpp"
#include "VFS.hpp"
#include "ACPI.hpp"

#include "KernelHeader.h"
#include "printf.h"

#define TIMEZONE -4

extern "C" void __attribute__((noreturn)) _start(KernelHeader* info)
{
	CMOS::RTC::UpdateBlocking();
	Screen  ::Init(info->screenBuffer, info->screenWidth, info->screenHeight, info->screenColorsInverted);
	Terminal::Init({0xBA,0xDA,0x55,0xFF}, {0x00,0x00,0x00,0xFF});
	Screen  ::Clear({0x00,0x00,0x00,0xFF});
	
	printf("Kernel's Position in memory: %016X\n", info->kernelImage.buffer);
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
	
	MemoryManager	::Init(info->physMapStart, info->pageBuffer, info->highMemoryBase	);
	GDT				::Init((uint64_t)info->kernelImage.buffer							);
	IDT				::Init(																);
	
	VFS::Init();
	
	ACPI::Init(info->RSDPStructure);
	
	Error::Panic("Reached end of main");
}