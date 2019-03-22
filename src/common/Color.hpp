#ifndef __COLOR_HPP__
#define __COLOR_HPP__

union Color
{
	struct
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
	} RGBA;
	struct
	{
		uint8_t blue;
		uint8_t green;
		uint8_t red;
		uint8_t alpha;
	} BGRA;
	uint32_t INT;
};

#endif