#include "Terminal.hpp"
#include "CMOS.hpp"
#include "Screen.hpp"

#include "kernel_header.h"
#include "printf.h"

#define TIMEZONE -4

static unsigned long int next = 1; 

unsigned long rand(void) // RAND_MAX assumed to be 32767 
{ 
    next = next * 1103515245 + 12345; 
    return (unsigned long)(next/65536); 
} 

void srand(unsigned int seed) 
{ 
    next = seed; 
} 

void PrintInfo()
{
	Terminal::SetCursor(0,0);
		
	printf("Screen Width: %d, Height: %d\n",
			Screen::Width(), Screen::Height()
			);
			
	printf("Char Count %d, %d\n", Terminal::CharWidth(), Terminal::CharHeight());
}

void PrintTime()
{
	CMOS::RTC::UpdateNoChecks();
	
	auto x = Terminal::X();
	auto y = Terminal::Y();
	
	Terminal::SetCursor(0,0);
	
	printf("Time: %2d:%02d:%02d%s %02d/%02d/%4d ",
			(CMOS::RTC::HourTimezone(TIMEZONE) % 12),
			CMOS::RTC::Minute(), CMOS::RTC::Second(), 
			(CMOS::RTC::HourTimezone(TIMEZONE) < 12) ? "AM" : "PM",
			CMOS::RTC::DayTimezone(TIMEZONE), 
			CMOS::RTC::MonthTimezone(TIMEZONE),
			CMOS::RTC::YearTimezone(TIMEZONE)
			);
			
	printf("Char Count %d, %d ", Terminal::CharWidth(), Terminal::CharHeight());
	
	//for (int i = Terminal::X(); i < Terminal::CharWidth(); i++)
		//printf(" ");
			
	Terminal::SetCursor(x,y);
}

uint32_t pixel = 4;

extern "C" void __attribute__((noreturn)) kernel_main(KernelHeader* info)
{
	Screen  ::Init(info->screenBuffer, info->screenWidth, info->screenHeight, info->screenColorsInverted);
	Terminal::Init({0x55,0xDA,0xBA,0xFF}, {0x00,0x00,0x00,0xFF});
	Screen  ::Clear({0x00,0x00,0x00,0xFF});
	
	CMOS::RTC::UpdateBlocking();
	
	//PrintInfo();
	
	srand(
		(CMOS::RTC::Day   () << 24) |
		(CMOS::RTC::Second() << 16) |
		(CMOS::RTC::Minute() <<  8) |
		(CMOS::RTC::Hour  () <<  0)
	);
	
	char l = 0;

	for (;;)
	{
		PrintTime();
		
		//printf("%c", l + 'A');
		//l = (l + 1) % 26;
		
		Color color;
		
		color.RGBA.red   = rand() % 0xFF;
		color.RGBA.green = rand() % 0xFF;
		color.RGBA.blue  = rand() % 0xFF;
		color.RGBA.alpha =          0xFF;
		uint32_t x       = rand() % (Screen::Width () / pixel);
		uint32_t y       = rand() % (Screen::Height() / pixel);
		
		for (int x2 = 0; x2 < pixel; x2++)
			for (int y2 = 0; y2 < pixel; y2++)
				Screen::SetPixel(color, x * pixel + x2, y * pixel + y2);		
	}

	asm("hlt");
}

