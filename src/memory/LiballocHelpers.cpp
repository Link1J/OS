#include "liballoc_1_1.h"
#include "MemoryManager.hpp"

bool lock;

int liballoc_lock()
{
	while (lock);
	lock = true;
	return 0;
}

int liballoc_unlock()
{
	lock = false;
	return 0;
}

void* liballoc_alloc(int pages)
{	
	return MemoryManager::AllocatePages(pages);
}

int liballoc_free(void* ptr, int pages)
{
	MemoryManager::FreePages(ptr, pages);
	return 0;
}