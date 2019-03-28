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

#include "KernelHeader.h"
#include "printf.h"

#define TIMEZONE -4

#define KernelMain _start

extern "C" [[noreturn]] void KernelMain(KernelHeader* info)
{
	CMOS::RTC::UpdateBlocking();
	Screen  ::Init(info->screenBuffer, info->screenWidth, info->screenHeight, info->screenColorsInverted);
	Terminal::Init({0xBA,0xDA,0x55,0xFF}, {0x00,0x00,0x00,0xFF});
	Screen  ::Clear({0x00,0x00,0x00,0xFF});
	
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
	
	MemoryManager	::Init(info->physMapStart, info->pageBuffer, info->highMemoryBase	);
	GDT				::Init((uint64_t)info->kernelImage.buffer							);
	IDT				::Init((uint64_t)info->kernelImage.buffer							);
	
	VFS::Init();
	PIC::Init();

	PIC::EnableKeyboardIRQ();
	new PS2Keyboard();

	auto file = VFS::OpenFile("/Devices/keyboard");

	asm("sti");

	while (true)
	{
		char buffer[10];
		int size = VFS::ReadFile(file, buffer, 10);
		for (int a = 0; a < size; a++)
		{
			printf("%02hhX ", buffer[a]);
		}
		if (size > 0)
			printf("\n");
	}

	ACPI::Init(info->RSDPStructure	);
		
	/*char buffer[50];
	
	printf("FileDescriptors\n");	
	auto file = VFS::OpenFile("/FileDescriptors");
	if (file == 0)
		printf("Failed to open\n");
	
	auto size = VFS::GetFileSize(file);
	
	for (int a = 0; a < size; a++)
	{
		VFS::ReadFile(file, buffer, 50);
		printf("\t%s\n", buffer);
	}*/
	
	/*
	*(char*)(0xDEADC0DE) = 100000;
	char a = *(char*)(0xDEADC0DE);
	*(char*)(0xDEADBEEF) = 100000;
	char b = *(char*)(0xDEADBEEF);
	//*/
	
	//Error::Panic("Reached end of main");
	for(;;) {
		asm("hlt");
	}
}