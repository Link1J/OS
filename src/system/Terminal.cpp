#include "Terminal.hpp"
#include "Font.hpp"
#include "printf.h"
#include "Screen.hpp"
#include "Vector2.hpp"

struct TerminalData
{
	Color	background;
	Color	foreground;
	Vector2	cursor;
	Vector2	charCount;
};

static TerminalData data;

void _putchar(char character)
{
	Terminal::PrintChar(character);
}

namespace Terminal
{
	float scale = 4;
	
	void Init(Color foregound, Color backgound)
	{
		data.cursor.x		= 0;
		data.cursor.y		= 0;
		data.background		= backgound;
		data.foreground		= foregound;
		data.charCount.x	= Screen::Width () / (float)((Font::Width () + 0) * scale);
		data.charCount.y	= Screen::Height() / (float)((Font::Height() + 1) * scale);
	}
	
	void SetCursor(int x, int y)
	{
		data.cursor.x = x;
		data.cursor.y = y;
	}
	
	void PrintChar(char c)
	{	 
		if ((c >= ' ' && c <= 127) || c == 0)
		{
			for (int xOffset = 0; xOffset < Font::Width(); xOffset++)
				for (int yOffset = 0; yOffset < Font::Height(); yOffset++)
					for (int xScale = 0; xScale < scale; xScale++)
						for (int yScale = 0; yScale < scale; yScale++)
						{
							int screenPosX = 
								((data.cursor.x * Font::Width () + xOffset                )
								* scale + xScale);
								
							int screenPosY = 
								((data.cursor.y * Font::Height() + yOffset + data.cursor.y) 
								* scale + yScale);
	
							Color color = Font::UseForground(c, xOffset, yOffset)
											? data.foreground
											: data.background;
	
							Screen::SetPixel(color, screenPosX, screenPosY);
						}
		
			if (data.cursor.x + 1 < data.charCount.x)
				data.cursor.x++;
			else
				NewLine();
		}
		else 
		{
			switch (c)
			{
			default  : PrintChar(0);      break;
			case '\n': NewLine();         break;
			case '\r': data.cursor.x = 0; break;
			case '\t': 
				{
					//int add = data.cursor.x % 4;
					//if (add == 0) add = 4;
					data.cursor.x += 4;//add;
					break;
				}
			}
		}
	}
	
	void SetForegoundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		data.foreground.RGBA.red	= red	;
		data.foreground.RGBA.green	= green	;
		data.foreground.RGBA.blue	= blue	;
	}
	
	void SetBackgoundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		data.background.RGBA.red	= red	;
		data.background.RGBA.green	= green	;
		data.background.RGBA.blue	= blue	;
	}
	
	void NewLine()
	{
		data.cursor.x = 0;
		data.cursor.y++;
				
		if (data.cursor.y >= data.charCount.y)
		{
			data.cursor.y--;
			for (int a = (Font::Height()) * scale; a < Screen::Height(); a++)
					Screen::CopyRow(a, a - (Font::Height() + 1) * scale);
				
			for (int a = Screen::Height() - (1 + Font::Height()) * scale; a < Screen::Height(); a++)
				Screen::ClearRow(data.background, a);
		}
	}
	
	uint32_t X         () { return data.cursor   .x; }
	uint32_t Y         () { return data.cursor   .y; }
	uint32_t CharWidth () { return data.charCount.x; }
	uint32_t CharHeight() { return data.charCount.y; }
}