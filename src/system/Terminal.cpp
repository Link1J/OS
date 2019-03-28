#include "Terminal.hpp"
#include "VFS.hpp"
#include "TTY/stdio.hpp"
#include "printf.h"
#include <string.h>
#include "MemoryUtils.hpp"

namespace Terminal
{
	static uint64_t posbuffer;
	static uint64_t pospath;
	static uint64_t maxPath;
	static uint64_t argCount;
	static char args[2][50];
	static char* buffer;
	static char* path;

	void Run()
	{
		auto stdio = stdio::File();
		posbuffer = 0;
		path = (char*)malloc(2);
		buffer = (char*)malloc(50);
		path[0] = '/';
		path[1] = '\0';
		pospath = 1;
		maxPath = 1;
		argCount = 0;
		
		while (true)
		{
			bool reading = true;

			VFS::WriteFile(stdio, path, pospath);
			VFS::WriteFile(stdio, (char*)"\n> ", 3);

			while (reading)
			{
				char miniBuf;
				int size = VFS::ReadFile(stdio, &miniBuf, 1);
				if (size > 0 && posbuffer < 50)
				{
					buffer[posbuffer] = miniBuf;
					posbuffer++;

					if (miniBuf == '\b')
					{
						buffer[--posbuffer] = 0;
						if (posbuffer > 0)
						{
							VFS::WriteFile(stdio, &miniBuf, 1);
							buffer[--posbuffer] = 0;
						}
						else if (argCount > 0)
						{
							VFS::WriteFile(stdio, &miniBuf, 1);
							memcpy(buffer, args[argCount-1], strlen(args[argCount-1]));
							argCount--;
							posbuffer = strlen(args[argCount]);
						}
					}
					else
					{
						VFS::WriteFile(stdio, &miniBuf, 1);
						if (miniBuf == '\n')
						{ 
							memcpy(args[argCount], buffer, posbuffer);
							args[argCount][posbuffer - 1] = 0;
							posbuffer = 0;
							argCount++;
							reading = false;
						}
						if (miniBuf == ' ')
						{
							memcpy(args[argCount], buffer, posbuffer);
							args[argCount][posbuffer - 1] = 0;
							posbuffer = 0;
							argCount++;
						}
					}
				}
			}
			//for (int a = 0; a < argCount; a++)
			//{
				//printf("%s ", args[a]);
			//}
			//printf("\n");

			if (memcmp(args[0], "ls", 3) == 0)
			{
				uint64_t file = VFS::OpenFile(path);
				if (file != 0)
				{
					uint64_t size = VFS::GetFileSize(file);

					char name[52];
					for (int a = 0; a < size; a++)
					{
						int number = VFS::ReadFile(file, name + 1, 50);
						name[0] = ' ';
						name[number] = '\n';
						VFS::WriteFile(stdio, name, number + 1);
					}

					VFS::CloseFile(file);
				}
			}
			else if (memcmp(args[0], "cd", 3) == 0)
			{
				if (argCount == 2)
				{
					if (memcmp(args[1], "..", 3) == 0)
					{
						while (path[pospath] != '/')
						{
							if (pospath > 0)
								pospath--;
						}
						if (pospath == 0)
							pospath++;
						path[pospath] = 0;
					}
					else
					{				
						int size = strlen(args[1]);
						if (pospath + size > maxPath)
						{
							char* temp = new char[maxPath + size + 1];
							memcpy(temp, path, maxPath);
							delete path;
							path = temp;
							maxPath = maxPath + size + 1;
						}
						char* temp = new char[maxPath];
						memcpy(temp, path, pospath);
						if (pospath > 1)
							temp[pospath++] = '/';
						memcpy(temp + pospath, args[1], size);
						temp[pospath + size] = 0;
						uint64_t file = VFS::OpenFolder(temp);
						if (file != 0)
						{
							memcpy(path, temp, maxPath);
							pospath += size;
						}
						else if (pospath > 1)
						{
							pospath--;
						}
						

						delete temp;
					}
				}
			}
			else if (memcmp(args[0], "clear", 6) == 0)
			{
				char help[2];
				memset(help, 0, 2);
				VFS::WriteFile(stdio, help, 2);
			}
			else
			{
				char* help = new char[27 + strlen(args[0]) + 1];
				snprintf(help, 27 + strlen(args[0]) + 1, "The command %s was not found\n", args[0]);
				VFS::WriteFile(stdio, help, 27 + strlen(args[0]));
			}
			

			argCount = 0;
		}
	}
	
	/*
	
	void Init()
	{
		InitSerial();
	}
	
	void SetCursor(int x, int y)
	{
		cursor.x = x;
		cursor.y = y;
	}
	
	void Hexdump(const char* data, int size)
	{
		printf("\t  %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX\n",
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
				
		
		for (int a = 0; a < size; a += 0x10)
		{
			char buffer[17];
			snprintf(buffer, 17, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 
				data[a +  0], data[a +  1], data[a +  2], data[a +  3],
				data[a +  4], data[a +  5], data[a +  6], data[a +  7],
				data[a +  8], data[a +  9], data[a + 10], data[a + 11],
				data[a + 12], data[a + 13], data[a + 14], data[a + 15]
				);
			
			printf("%04llX: ", a);
			
			printf("%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX",
				data[a +  0], data[a +  1], data[a +  2], data[a +  3],
				data[a +  4], data[a +  5], data[a +  6], data[a +  7],
				data[a +  8], data[a +  9], data[a + 10], data[a + 11],
				data[a + 12], data[a + 13], data[a + 14], data[a + 15]
				);
				
			printf("\t\t|");	
			
			for (int b = 0; b < 16; b++)
				Terminal::PrintSymbol(buffer[b]);
				
			printf("|\n");
		}
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
	
	uint32_t X         () { return cursor   .x; }
	uint32_t Y         () { return cursor   .y; }
	uint32_t CharWidth () { return charCount.x; }
	uint32_t CharHeight() { return charCount.y; }*/
}