#include "TTY.hpp"
#include "printf.h"
#include "Terminal.hpp"
#include "VFS.hpp"


#include "Font3x5.hpp"
#include "FontPC.hpp"
namespace Font = FontPC;

TTY::TTY(const char* name) : Device("tty", name) 
{
}

uint64_t TTY::Read(uint64_t pos, void* buffer, uint64_t bufferSize)
{
}

void TTYScreen::PrintChar(char c)
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
	
void TTYScreen::PrintSymbol(char c)
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

void TTYScreen::NewLine()
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

TTYScreen::TTYScreen(Color foregound, Color backgound) : TTY("screen")
{
    cursor.x	= 0;
	cursor.y	= 0;
	background	= backgound;
	foreground	= foregound;
	charCount.x	= Screen::Width () / (float)((Font::Width () + 0) * scale);
	charCount.y	= Screen::Height() / (float)((Font::Height() + 1) * scale);
}

void TTYScreen::Write(uint64_t pos, void* buffer, uint64_t bufferSize)
{
    for (int a = 0; a < bufferSize; a++)
        Terminal::PrintChar(((char*)buffer)[0]);
}

