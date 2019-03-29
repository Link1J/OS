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
	static char args[2][51];
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
				if (size > 0)
				{
					if (miniBuf == '\b')
					{
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
						bool flag = false;
						if (miniBuf == '\n')
						{ 
							memcpy(args[argCount], buffer, posbuffer);
							args[argCount][posbuffer] = 0;
							posbuffer = 0;
							argCount++;
							reading = false;
							flag = true;
						}
						if (miniBuf == ' ')
						{
							memcpy(args[argCount], buffer, posbuffer);
							args[argCount][posbuffer] = 0;
							posbuffer = 0;
							argCount++;
							flag = true;
						}

						if (posbuffer < 50)
						{
							VFS::WriteFile(stdio, &miniBuf, 1);
							if (!flag)
							{
								buffer[posbuffer] = miniBuf;
								posbuffer++;
							}
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
							VFS::CloseFile(file);
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
				int length = strlen(args[0]);
				if (length > 50)
					length = 50;

				char* help = new char[27 + length + 1];
				snprintf(help, 27 + length + 1, "The command %s was not found\n", args[0]);
				VFS::WriteFile(stdio, help, 27 + length);
			}
			

			argCount = 0;
		}
	}
}