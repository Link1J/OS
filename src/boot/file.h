#ifndef __FILE_H__
#define __FILE_H__

#include <stdint.h>

struct _FILE_DATA
{
	uint64_t	size;
	uintptr_t	data;
};
typedef struct _FILE_DATA FILE_DATA;

FILE_DATA openFile(const wchar_t* path);

#endif