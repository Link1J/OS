#include <cstdint>

namespace CMOS
{
	namespace RTC
	{
		void Update        ();
		void UpdateNoChecks();
		void UpdateBlocking();
		
		uint8_t  Second();
		uint8_t  Minute();
		uint8_t  Hour  ();
		uint8_t  Day   ();
		uint8_t  Month ();
		uint32_t Year  ();
		
		uint8_t  HourTimezone (int8_t timezone);
		uint8_t  DayTimezone  (int8_t timezone);
		uint8_t  MonthTimezone(int8_t timezone);
		uint32_t YearTimezone (int8_t timezone);
	}
}
