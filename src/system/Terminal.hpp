#include <cstdint>
#include "Color.hpp"

namespace Terminal
{
	void Init(Color foregound, Color backgound);

	void SetCursor			(int x, int y								);
	void PrintChar			(char c										);
	void PrintSymbol		(char c										);
	void SetForegoundColor	(uint8_t red, uint8_t green, uint8_t blue	);
	void SetBackgoundColor	(uint8_t red, uint8_t green, uint8_t blue	);
	
	void NewLine();
	
	uint32_t X         ();
	uint32_t Y         ();
	uint32_t CharWidth ();
	uint32_t CharHeight();
};