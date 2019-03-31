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
	static uint64_t stdio;
	static char args[2][51];
	static char* argBuffer;
	static char* path;

	static uint8_t colors[] = 
	{
		0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x64, 0xFF, 0xFF,
		0xFF, 0x40, 0x40, 0xFF,

		0xBA, 0xDA, 0x55, 0xFF,
	};

	void ReadAllByte(uint64_t file)
	{
		uint64_t size = VFS::GetFileSize(file);

		char* buffer = new char[3];
		int number = 0;
		if (size > 0)
		{
			do
			{
				number = VFS::ReadFile(file, buffer, 1);
				if (buffer[0] == 0)
					buffer[0] = ' ';
				if (number > 0)
					VFS::WriteFile(stdio, buffer, 1);
			}
			while (number > 0);

			if (buffer[0] != '\n')
			{
				buffer[0] = '\n';
				VFS::WriteFile(stdio, buffer, 1);
			}
		}

		delete[] buffer;
		VFS::CloseFile(file);
	}

	void Run()
	{
		stdio = stdio::File();
		posbuffer = 0;
		path = (char*)malloc(2);
		argBuffer = (char*)malloc(50);
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
							argBuffer[--posbuffer] = 0;
						}
						else if (argCount > 0)
						{
							VFS::WriteFile(stdio, &miniBuf, 1);
							memcpy(argBuffer, args[argCount-1], strlen(args[argCount-1]));
							argCount--;
							posbuffer = strlen(args[argCount]);
						}
					}
					else
					{
						if (argCount < 2)
						{
							bool flag = false;
							if (miniBuf == '\n')
							{ 
								memcpy(args[argCount], argBuffer, posbuffer);
								args[argCount][posbuffer] = 0;
								posbuffer = 0;
								argCount++;
								reading = false;
								flag = true;
							}
							if (miniBuf == ' ')
							{
								memcpy(args[argCount], argBuffer, posbuffer);
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
									argBuffer[posbuffer] = miniBuf;
									posbuffer++;
								}
							}
						}
					}
				}
			}
			
			if (memcmp(args[0], "ls", 3) == 0)
			{
				uint64_t file = VFS::OpenFile(path);
				if (file != 0)
				{
					uint64_t size = VFS::GetFileSize(file);

					char name[54];
					char* help = new char[200];
					int type = 0;

					for (int a = 0; a < size / 50; a++)
					{
						int number = VFS::ReadFile(file, name + 3, 50);
						if (pospath > 1)
							snprintf(help, 200, "%s/%s", path, name + 3);
						else
							snprintf(help, 200, "%s%s", path, name + 3);
						uint64_t tempfile = VFS::OpenFile(help);
						int type = (int)VFS::GetType(tempfile);
						VFS::CloseFile(tempfile);

						help[0] = 0;
						help[1] = 1;
						memcpy(help + 2, colors + type * 4, 4);
						VFS::WriteFile(stdio, help, 6);

						name[0] = '\t';
						name[1] = '0' + type;
						name[2] = ' ';
						name[number + 2] = '\n';
						VFS::WriteFile(stdio, name, number + 3);
					}
					
					help[0] = 0;
					help[1] = 1;
					memcpy(help + 2, colors + 3 * 4, 4);
					VFS::WriteFile(stdio, help, 6);

					delete[] help;
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
							delete[] path;
							path = temp;
							maxPath = maxPath + size + 2;
						}

						char* temp = new char[maxPath];
						memcpy(temp, path, pospath);
						if (pospath > 1)
							temp[pospath++] = '/';
						memcpy(temp + pospath, args[1], size + 1);
						
						uint64_t file = VFS::OpenFolder(temp);
						if (file != 0)
						{
							memcpy(path, temp, maxPath + 1);
							pospath += size;
							VFS::CloseFile(file);
						}
						else if (pospath > 1)
						{
							pospath--;
						}

						delete[] temp;
					}
				}
			}
			else if (memcmp(args[0], "read", 4) == 0)
			{
				if (argCount > 1)
				{
					for (int a = 1; a < argCount; a++)
					{
						uint64_t length = strlen(args[a]);
						char* buffer = new char[pospath + length + 2];
						if (pospath > 1)
							snprintf(buffer, pospath + length + 2, "%s/%s", path, args[a]);
						else
							snprintf(buffer, pospath + length + 2, "%s%s", path, args[a]);
						uint64_t file = VFS::OpenFile(buffer);
						delete[] buffer;

						if (file != 0)
						{
							ReadAllByte(file);
						}
						else
						{
							buffer = new char[18 + length];
							length = snprintf(buffer, 18 + length, "Could not open %s.\n", args[a]);
							VFS::WriteFile(stdio, buffer, length);
							delete[] buffer;
						}
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
				delete[] help;
			}		

			argCount = 0;
		}
	}
}