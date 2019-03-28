#include "Screen.hpp"
#include "Vector2.hpp"

struct ScreenData
{
	Color*	screenBuffer;
	Vector2	screenSize;
	bool    invertColors;
};

static ScreenData data;

namespace Screen
{
	void Init(uint32_t* videoBuffer, int width, int height, bool invertColors)
	{
		data.screenBuffer	= (Color*)videoBuffer;
		data.screenSize.x	= width;
		data.screenSize.y	= height;
	}
	
	void SetPixel(Color color, int x, int y)
	{
		uint64_t index = x + y * data.screenSize.x;
		
		if (index >= data.screenSize.x * data.screenSize.y)
			return;

		Color curr = data.screenBuffer[index];
		
		curr.RGBA.red	 = (curr.RGBA.red	 / 256. * (1 - color.RGBA.alpha / 256.)) * 256;
		curr.RGBA.green	 = (curr.RGBA.green	 / 256. * (1 - color.RGBA.alpha / 256.)) * 256;
		curr.RGBA.blue	 = (curr.RGBA.blue	 / 256. * (1 - color.RGBA.alpha / 256.)) * 256;
		
		color.RGBA.red	 = (color.RGBA.red	 / 256. * (    color.RGBA.alpha / 256.)) * 256;
		color.RGBA.green = (color.RGBA.green / 256. * (    color.RGBA.alpha / 256.)) * 256;
		color.RGBA.blue	 = (color.RGBA.blue	 / 256. * (    color.RGBA.alpha / 256.)) * 256;
		
		color.RGBA.red	 = color.RGBA.red	+ curr.RGBA.red		;
		color.RGBA.green = color.RGBA.green	+ curr.RGBA.green	;
		color.RGBA.blue	 = color.RGBA.blue	+ curr.RGBA.blue	;
		
		if (!data.invertColors)
		{
			curr = color;
			
			color.BGRA.red		= curr.RGBA.red		;
			color.BGRA.green	= curr.RGBA.green	;
			color.BGRA.blue		= curr.RGBA.blue	;
		}
		
		data.screenBuffer[index] = color;
	}
	
	void Clear(Color color)
	{
		for (int y = 0; y < data.screenSize.y; y++)
			ClearRow(color, y);
	}
	
	void ClearRow(Color color, int row)
	{
		int rowStart = row * data.screenSize.x;
		for (int x = 0; x < data.screenSize.x; x++)
			data.screenBuffer[rowStart + x] = color;
	}
	
	void CopyRow(int from, int to)
	{
		int rowStartFrom = from * data.screenSize.x;
		int rowStartTo   = to   * data.screenSize.x;
		
		for (int x = 0; x < data.screenSize.x; x++)
			data.screenBuffer[rowStartTo + x] = data.screenBuffer[rowStartFrom + x];
	}
	
	
	int Width () { return data.screenSize.x; }
	int Height() { return data.screenSize.y; }
}