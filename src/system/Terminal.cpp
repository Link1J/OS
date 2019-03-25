#include "Terminal.hpp"
#include "Font3x5.hpp"
#include "FontPC.hpp"
#include "printf.h"
#include "Screen.hpp"
#include "Vector2.hpp"

namespace Font = FontPC;

void _putchar(char character)
{
	Terminal::PrintChar(character);
}

namespace Terminal
{
	float scale = 1;
	Color	background;
	Color	foreground;
	Vector2	cursor;
	Vector2	charCount;
	
	void Init(Color foregound, Color backgound)
	{
		cursor.x	= 0;
		cursor.y	= 0;
		background	= backgound;
		foreground	= foregound;
		charCount.x	= Screen::Width () / (float)((Font::Width () + 0) * scale);
		charCount.y	= Screen::Height() / (float)((Font::Height() + 1) * scale);
	}
	
	void SetCursor(int x, int y)
	{
		cursor.x = x;
		cursor.y = y;
	}
	
	void PrintChar(char c)
	{	 
		if ((c >= ' ' && c <= 127) || c == 0)
		{
			PrintSymbol(c);
		}
		else 
		{
			switch (c)
			{
			default  : PrintSymbol(0);	break;
			case '\n': NewLine();		break;
			case '\r': cursor.x = 0;	break;
			case '\t': 
				{
					int add = cursor.x % 4;
					if (add == 0) add = 4;
					cursor.x += add;
					break;
				}
			}
		}
	}
	
	void PrintSymbol(char c)
	{
		for (int xOffset = 0; xOffset < Font::Width(); xOffset++)
			for (int yOffset = 0; yOffset < Font::Height(); yOffset++)
				for (int xScale = 0; xScale < scale; xScale++)
					for (int yScale = 0; yScale < scale; yScale++)
					{
						int screenPosX = 
							((cursor.x * Font::Width () + xOffset           )
							* scale + xScale);
							
						int screenPosY = 
							((cursor.y * Font::Height() + yOffset + cursor.y) 
							* scale + yScale);
	
						Color color = Font::UseForground(c, xOffset, yOffset)
									? foreground
									: background;

						Screen::SetPixel(color, screenPosX, screenPosY);
					}
		
		if (cursor.x + 1 < charCount.x)
			cursor.x++;
		else
			NewLine();
	}
	
	void SetForegoundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		foreground.RGBA.red		= red	;
		foreground.RGBA.green	= green	;
		foreground.RGBA.blue	= blue	;
	}
	
	void SetBackgoundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		background.RGBA.red		= red	;
		background.RGBA.green	= green	;
		background.RGBA.blue	= blue	;
	}
	
	void NewLine()
	{
		cursor.x = 0;
		cursor.y++;
				
		if (cursor.y >= charCount.y)
		{
			cursor.y--;
			for (int a = (Font::Height()) * scale; a < Screen::Height(); a++)
					Screen::CopyRow(a, a - (Font::Height() + 1) * scale);
				
			for (int a = Screen::Height() - (1 + Font::Height()) * scale; a < Screen::Height(); a++)
				Screen::ClearRow(background, a);
		}
	}
	
	uint32_t X         () { return cursor   .x; }
	uint32_t Y         () { return cursor   .y; }
	uint32_t CharWidth () { return charCount.x; }
	uint32_t CharHeight() { return charCount.y; }
}