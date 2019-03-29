#include "Terminal.hpp"
#include "VFS.hpp"
#include "TTY/stdio.hpp"
#include "printf.h"
#include <string.h>
#include "MemoryUtils.hpp"

namespace Terminal
{
	struct Arg
	{
		char* data;
		uint64_t max;
		uint64_t curr;

		Arg()
		{
			data = new char[1];
			max = 1;
			curr = 0;
		}

		void IncressMax(int size = 1)
		{
			char* temp = new char[max + size];
			memcpy(temp, data, max);
			delete data;
			data = temp;
			max += size;
		}

		void ZeroCurr()
		{
			curr = 0;
		}

		char& operator[](int idx)
		{
			return data[idx];
		}
	};

	struct Args
	{
		Arg* args;
		uint64_t max;
		uint64_t count;

		Arg& operator[](int idx)
		{
			args[idx];
		}

		void IncressMax(int size = 1)
		{
			Arg* temp = new Arg[max + size];
			memcpy(temp, args, max);
			delete args;
			args = temp;
			max += size;
		}
	};

	static Args args;
	static Arg path;

	void Run()
	{
		auto stdio = stdio::File();
		path.data = new char[2];
		memcpy(path.data, "/", 2);
		path.max = path.curr = 1;
		args.args = new Arg[1];
		args.max = 1;
		args.count = 0;
		
		while (true)
		{
			bool reading = true;

			VFS::WriteFile(stdio, path.data, path.curr);
			VFS::WriteFile(stdio, (char*)"\n> ", 3);

			while (reading)
			{
				char miniBuf;
				int size = VFS::ReadFile(stdio, &miniBuf, 1);
				if (size > 0)
				{
					Arg& curr = args[args.count];
					if (curr.curr == curr.max)
						curr.IncressMax();
					curr[curr.curr] = miniBuf;
					curr.curr++;

					if (miniBuf == '\b')
					{
						curr[--curr.curr] = 0;
						if (curr.curr > 0)
						{
							VFS::WriteFile(stdio, &miniBuf, 1);
							curr[--curr.curr] = 0;
						}
						else if (args.count > 0)
						{
							args.count--;
						}
					}
					else
					{
						VFS::WriteFile(stdio, &miniBuf, 1);
						if (miniBuf == '\n')
						{ 
							args.count++;
							if (args.count == args.max)
								args.IncressMax();
							reading = false;
						}
						if (miniBuf == ' ')
						{
							args.count++;
							if (args.count == args.max)
								args.IncressMax();
						}
					}
				}
			}
			//for (int a = 0; a < argCount; a++)
			//{
				//printf("%s ", args[a]);
			//}
			//printf("\n");

			if (memcmp(args[0].data, "ls", 3) == 0)
			{
				uint64_t file = VFS::OpenFile(path.data);
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
			else if (memcmp(args[0].data, "cd", 3) == 0)
			{
				if (args.count == 2)
				{
					if (memcmp(args[1].data, "..", 3) == 0)
					{
						while (path[path.curr] != '/')
						{
							if (path.curr > 0)
								path.curr--;
						}
						if (path.curr == 0)
							path.curr++;
						path[path.curr] = 0;
					}
					else
					{				
						int size = strlen(args[1].data);
						if (path.curr + size > path.max)
							path.IncressMax(size + 1);
						char* temp = new char[path.max];
						memcpy(temp, path.data, path.curr);
						if (path.curr > 1)
							temp[path.curr++] = '/';
						memcpy(temp + path.curr, args[1].data, size);
						temp[path.curr + size] = 0;
						uint64_t file = VFS::OpenFolder(temp);
						if (file != 0)
						{
							memcpy(path.data, temp, path.max);
							path.curr += size;
						}
						else if (path.curr > 1)
						{
							path.curr--;
						}

						delete temp;
					}
				}
			}
			else if (memcmp(args[0].data, "clear", 6) == 0)
			{
				char help[2];
				memset(help, 0, 2);
				VFS::WriteFile(stdio, help, 2);
			}
			else
			{
				char* help = new char[27 + strlen(args[0].data) + 1];
				snprintf(help, 27 + strlen(args[0].data) + 1, "The command %s was not found\n", args[0].data);
				VFS::WriteFile(stdio, help, 27 + strlen(args[0].data));
			}
			
			for (int a = 0; a < args.count; a++)
				args[a].ZeroCurr();
			args.count = 0;
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