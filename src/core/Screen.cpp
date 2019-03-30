#include "Screen.hpp"
#include "Vector2.hpp"
#include "printf.h"

namespace Screen
{
	static Color*	screenBuffer;
	static Vector2	screenSize;
	static bool    invertColors;

	void Init(uint32_t* videoBuffer, int width, int height, bool invertColors)
	{
		screenBuffer	= (Color*)videoBuffer;
		screenSize.x	= width;
		screenSize.y	= height;
	}
	
	void SetPixel(Color color, int x, int y)
	{
		uint64_t index = x + y * screenSize.x;

		if (index >= screenSize.x * screenSize.y)
			return;
		if ((uint64_t)screenBuffer < 0xFFFFFF0000000000)
			return;

		Color curr = screenBuffer[index];
		
		curr.RGBA.red	 = (curr.RGBA.red	 / 256. * (1 - color.RGBA.alpha / 256.)) * 256;
		curr.RGBA.green	 = (curr.RGBA.green	 / 256. * (1 - color.RGBA.alpha / 256.)) * 256;
		curr.RGBA.blue	 = (curr.RGBA.blue	 / 256. * (1 - color.RGBA.alpha / 256.)) * 256;
		
		color.RGBA.red	 = (color.RGBA.red	 / 256. * (    color.RGBA.alpha / 256.)) * 256;
		color.RGBA.green = (color.RGBA.green / 256. * (    color.RGBA.alpha / 256.)) * 256;
		color.RGBA.blue	 = (color.RGBA.blue	 / 256. * (    color.RGBA.alpha / 256.)) * 256;
		
		color.RGBA.red	 = color.RGBA.red	+ curr.RGBA.red		;
		color.RGBA.green = color.RGBA.green	+ curr.RGBA.green	;
		color.RGBA.blue	 = color.RGBA.blue	+ curr.RGBA.blue	;
		
		if (!invertColors)
		{
			curr = color;
			
			color.BGRA.red		= curr.RGBA.red		;
			color.BGRA.green	= curr.RGBA.green	;
			color.BGRA.blue		= curr.RGBA.blue	;
		}
		
		screenBuffer[index] = color;
	}
	
	void Clear(Color color)
	{
		for (int y = 0; y < screenSize.y; y++)
			ClearRow(color, y);
	}
	
	void ClearRow(Color color, int row)
	{
		int rowStart = row * screenSize.x;
		for (int x = 0; x < screenSize.x; x++)
			screenBuffer[rowStart + x] = color;
	}
	
	void CopyRow(int from, int to)
	{
		int rowStartFrom = from * screenSize.x;
		int rowStartTo   = to   * screenSize.x;
		
		for (int x = 0; x < screenSize.x; x++)
			screenBuffer[rowStartTo + x] = screenBuffer[rowStartFrom + x];
	}
	
	
	int Width () { return screenSize.x; }
	int Height() { return screenSize.y; }
}