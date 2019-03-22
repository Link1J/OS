#include "CMOS.hpp"
#include "IO.hpp"

#define CURRENT_YEAR 2010									// Change this each year!
 
int century_register = 0x00;								// Set by ACPI table parsing code if possible
 
uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint32_t year;
uint8_t century;
 
enum {
      cmos_address = 0x70,
      cmos_data    = 0x71
};
 
int get_update_in_progress_flag() {
      IO::Out::Byte(cmos_address, 0x0A);
      return (IO::In::Byte(cmos_data) & 0x80);
}
 
unsigned char get_RTC_register(int reg) {
      IO::Out::Byte(cmos_address, reg);
      return IO::In::Byte(cmos_data);
}
 
namespace CMOS
{
	namespace RTC
	{
		void UpdateVarablesFromPorts()
		{
			second	= get_RTC_register(0x00);
			minute	= get_RTC_register(0x02);
			hour	= get_RTC_register(0x04);
			day		= get_RTC_register(0x07);
			month	= get_RTC_register(0x08);
			year	= get_RTC_register(0x09);

			if(century_register != 0) 
			{
				century = get_RTC_register(century_register);
			}
		}
		
		void UpdateVarablesWithMath()
		{
			uint8_t registerB = get_RTC_register(0x0B);
 
			// Convert BCD to binary values if necessary
 
			if (!(registerB & 0x04)) 
			{
				second	= (second & 0x0F) + ((second         / 16) * 10)                 ;
				minute	= (minute & 0x0F) + ((minute         / 16) * 10)                 ;
				hour	= (( hour & 0x0F) + ((( hour & 0x70) / 16) * 10)) | (hour & 0x80);
				day		= (   day & 0x0F) + ((   day         / 16) * 10)                 ;
				month	= ( month & 0x0F) + (( month         / 16) * 10)                 ;
				year	= (  year & 0x0F) + ((  year         / 16) * 10)                 ;
				
				if(century_register != 0) 
				{
					century = (century & 0x0F) + ((century / 16) * 10);
				}
			}
 
			// Convert 12 hour clock to 24 hour clock if necessary
 
			if (!(registerB & 0x02) && (hour & 0x80)) 
			{
				hour = ((hour & 0x7F) + 12) % 24;
			}
 
			// Calculate the full (4-digit) year
 
			if(century_register != 0) 
			{
				year += century * 100;
			} 
			else 
			{
				year += (CURRENT_YEAR / 100) * 100;
				if(year < CURRENT_YEAR) year += 100;
			}
		}
		
		void Update()
		{
			while (get_update_in_progress_flag());				// Make sure an update isn't in progress
			UpdateNoChecks();
		}
		
		void UpdateNoChecks() 
		{
			UpdateVarablesFromPorts();
			UpdateVarablesWithMath();
		}
		
		void UpdateBlocking() 
		{
			unsigned char century;
			unsigned char last_second;
			unsigned char last_minute;
			unsigned char last_hour;
			unsigned char last_day;
			unsigned char last_month;
			unsigned char last_year;
			unsigned char last_century;
			unsigned char registerB;
 
			// Note: This uses the "read registers until you get the same values twice in a row" technique
			//       to avoid getting dodgy/inconsistent values due to RTC updates
			
			while (get_update_in_progress_flag());				// Make sure an update isn't in progress
			UpdateVarablesFromPorts();
			
			do 
			{
				last_second = second;
				last_minute = minute;
				last_hour = hour;
				last_day = day;
				last_month = month;
				last_year = year;
				last_century = century;
 
				while (get_update_in_progress_flag());			// Make sure an update isn't in progress
				UpdateVarablesFromPorts();
			} 
			while(
				(last_second != second) || (last_minute != minute) || (last_hour != hour) ||
				(last_day != day) || (last_month != month) || (last_year != year) ||
				(last_century != century) 
				);
 
			UpdateVarablesWithMath();
		}
		
		uint8_t  Second() { return second; }
		uint8_t  Minute() { return minute; }
		uint8_t  Hour  () { return hour  ; }
		uint8_t  Day   () { return day   ; }
		uint8_t  Month () { return month ; }
		uint32_t Year  () { return year  ; }
		
		uint8_t HourTimezone(int8_t timezone) 
		{ 
			uint8_t hourTimezone = hour + timezone;
			
			if (timezone < 0 && hourTimezone < 0)
			{
				hourTimezone = hourTimezone + 24;
			}
			if (timezone > 0 && hourTimezone > 0)
			{
				hourTimezone = hourTimezone - 24;
			}
		
			return hourTimezone; 
		}
		
		uint8_t DayTimezone(int8_t timezone) 
		{ 
			uint8_t hourTimezone = HourTimezone(timezone);
			
			if (hourTimezone > hour && timezone < 0)
				return day - 1;
			else if (hourTimezone < hour && timezone > 0)
				return day + 1;
			else
				return day;
		}
		
		uint8_t MonthTimezone(int8_t timezone) 
		{ 
			uint8_t dayTimezone = DayTimezone(timezone);
			
			if (dayTimezone > day && timezone < 0)
				return month - 1;
			else if (dayTimezone < day && timezone > 0)
				return month + 1;
			else
				return month;
		}
		
		uint32_t YearTimezone(int8_t timezone) 
		{ 
			uint8_t monthTimezone = MonthTimezone(timezone);
			
			if (monthTimezone > month && timezone < 0)
				return year - 1;
			else if (monthTimezone < month && timezone > 0)
				return year + 1;
			else
				return year;
		}
	}
}