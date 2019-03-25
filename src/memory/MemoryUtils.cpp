#include <stddef.h>
#include "KernelHeap.hpp"

extern "C" void* memcpy(void* dsc, void* src, size_t n)
{
	for (size_t a = 0; a < n; a++)
	{
		((char*)dsc)[a] = ((char*)src)[a];
	}
	
	return dsc;
}

extern "C" void* memset(void* dsc, int val, size_t n)
{
	for (size_t a = 0; a < n; a++)
	{
		((char*)dsc)[a] = val;
	}
	
	return dsc;
}

extern "C" int memcmp(const void *s1, const void *s2, size_t n)
{
	const char* cs = (const char*)s1;
	const char* ct = (const char*)s2;
	
	for (size_t i = 0; i < n; i++, cs++, ct++)
		if (*cs < *ct)
			return -1;
		else if (*cs > *ct)
			return 1;
	return 0;
}

extern "C" void* malloc(size_t size)
{
	return KernelHeap::Allocate(size);
}

extern "C" void free(void* block)
{
	KernelHeap::Free(block);
}

void* operator new(size_t size)
{
    return malloc(size);
}

void* operator new[](size_t size)
{
    return operator new(size);
}

void operator delete(void* block)
{
    free(block);
}

void operator delete(void* block, size_t size)
{
    operator delete(block);
}

void operator delete[](void* block, size_t size)
{
    operator delete(block);
}

void operator delete[](void* block)
{
    operator delete(block);
}