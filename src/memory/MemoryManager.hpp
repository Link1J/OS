#include "KernelHeader.h"

#include <cstdint>

namespace MemoryManager
{
	void	Init(PhysicalMapSegment* physMapStart, uint64_t* pageBuffer, uint64_t highMemoryBase);
	
	void*	PhysicalToKernelPtr(void* ptr);
    void*	KernelToPhysicalPtr(void* ptr);
	
	void*	AllocatePages	(uint64_t numPages				);
	void	FreePages		(void* pages, uint64_t numPages	);
	
	void MapKernelPage	(void* phys, void* virt	);
    void UnmapKernelPage(void* virt				);
	
	void* GetPhysicalFromVirtual(void* virt);
}