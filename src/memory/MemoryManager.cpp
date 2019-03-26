#include "MemoryManager.hpp"
#include "MemoryList.hpp"
#include "Error.hpp"

#include "printf.h"

#define CEILING_POS(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))
#define CEILING_NEG(X) ((X-(int)(X)) < 0 ? (int)(X-1) : (int)(X))
#define CEILING(X)  (((X) > 0) ? CEILING_POS(X) : CEILING_NEG(X))

#define PML_GET_NX(entry)           ((entry) & 0x8000000000000000)
#define PML_GET_ADDR(entry)         ((entry) & 0x000FFFFFFFFFF000)

#define PML_SET_NX                  0x8000000000000000
#define PML_SET_ADDR(addr)          (addr)

#define PML_GET_A(entry)            ((entry) & 0x20)
#define PML_GET_PCD(entry)          ((entry) & 0x10)
#define PML_GET_PWT(entry)          ((entry) & 0x8)
#define PML_GET_US(entry)           ((entry) & 0x4)
#define PML_GET_RW(entry)           ((entry) & 0x2)
#define PML_GET_P(entry)            ((entry) & 0x1)

#define PML_SET_A(a)                ((a) ? 0x20 : 0)
#define PML_SET_PCD(a)              ((a) ? 0x10 : 0)
#define PML_SET_PWT(a)              ((a) ? 0x8 : 0)
#define PML_SET_US(a)               ((a) ? 0x4 : 0)
#define PML_SET_RW(a)               ((a) ? 0x2 : 0)
#define PML_SET_P(a)                ((a) ? 0x1 : 0)

#define PML1_GET_G(entry)           ((entry) & 0x100)
#define PML1_GET_PAT(entry)         ((entry) & 0x80)
#define PML1_GET_D(entry)           ((entry) & 0x40)

#define PML1_SET_G(a)               ((a) ? 0x100 : 0)
#define PML1_SET_PAT(a)             ((a) ? 0x80 : 0)
#define PML1_SET_D(a)               ((a) ? 0x40 : 0)

#define GET_PML1_INDEX(addr)        (((addr) >> (12 + 0 * 9)) & 0x1FF)
#define GET_PML2_INDEX(addr)        (((addr) >> (12 + 1 * 9)) & 0x1FF)
#define GET_PML3_INDEX(addr)        (((addr) >> (12 + 2 * 9)) & 0x1FF)
#define GET_PML4_INDEX(addr)        (((addr) >> (12 + 3 * 9)) & 0x1FF)


namespace MemoryManager
{	
	static MemoryList	freeMemory;
	static uint64_t		highMemoryBase;
	static uint64_t*	pageTableLevel4;
	
	
	void Init(PhysicalMapSegment* physMapStart, uint64_t* pageBuffer, uint64_t highMemoryBaseIn)
	{
		printf("Starting Memory manager\n");
		
		freeMemory = MemoryList();
		
		highMemoryBase = highMemoryBaseIn;
		pageTableLevel4 = pageBuffer;

		// delete lower half mapping
		pageTableLevel4[0] = 0;

		PhysicalMapSegment* curr = physMapStart;
		PhysicalMapSegment* next = nullptr;
		uint64_t size = 0;
		uint64_t availableMemory = 0;
		
		while (curr != nullptr)
		{	
			size = curr->numPages * 4096;
			next = curr->next;
			
			freeMemory.MarkFree(curr, size);
			
			curr = next;
			availableMemory += size;
		}

		uint64_t* heapPageTableLevel3 = (uint64_t*)PhysicalToKernelPtr(AllocatePages(1));
		for(int i = 0; i < 512; i++)
			heapPageTableLevel3[i] = 0;
		pageTableLevel4[510] = PML_SET_ADDR((uint64_t)KernelToPhysicalPtr(heapPageTableLevel3)) | PML_SET_P(1) | PML_SET_RW(1);

		double temp = availableMemory;
		for (int type = 0; type < 2; type++)
			temp /= 1024;
		availableMemory = CEILING(temp);
		
		printf("Available memory: %d MB\n", availableMemory);
		printf("Memory manager initialized\n");
	}
	
	void* PhysicalToKernelPtr(void* ptr)
	{
        return (char*)ptr + highMemoryBase;
    }

    void* KernelToPhysicalPtr(void* ptr)
    {
        return (char*)ptr - highMemoryBase;
    }

	void* AllocatePages(uint64_t numPages)
    {
		//if (freeMemory.Size() == 0)
			//Error::Panic("Out of memory!");
		
		void* mem = freeMemory.FindFree(numPages * 4096);
		if (mem != nullptr)
		{
			freeMemory.MarkUsed(mem, numPages * 4096);
			mem = KernelToPhysicalPtr(mem);
		}
		
        return mem;
    }

    void FreePages(void* pages, uint64_t numPages)
    {
		freeMemory.MarkFree(pages, numPages * 4096);
    }
	
	void MapKernelPage(void* phys, void* virt)
    {
		uint64_t pml4Index = GET_PML4_INDEX((uint64_t)virt);
        uint64_t pml3Index = GET_PML3_INDEX((uint64_t)virt);
        uint64_t pml2Index = GET_PML2_INDEX((uint64_t)virt);
        uint64_t pml1Index = GET_PML1_INDEX((uint64_t)virt);

        uint64_t pml4Entry = pageTableLevel4[pml4Index];
        uint64_t* pml3 = (uint64_t*)PhysicalToKernelPtr((void*)PML_GET_ADDR(pml4Entry));

        uint64_t pml3Entry = pml3[pml3Index];
        uint64_t* pml2;
        if(!PML_GET_P(pml3Entry)) {
            pml2 = (uint64_t*)PhysicalToKernelPtr(AllocatePages(1));
            for(int i = 0; i < 512; i++)
                pml2[i] = 0;
            pml3[pml3Index] = PML_SET_ADDR((uint64_t)KernelToPhysicalPtr(pml2)) | PML_SET_P(1) | PML_SET_RW(1);
        } else {
            pml2 = (uint64_t*)PhysicalToKernelPtr((void*)PML_GET_ADDR(pml3Entry));
        }

        uint64_t pml2Entry = pml2[pml2Index];
        uint64_t* pml1;
        if(!PML_GET_P(pml2Entry)) {
            pml1 = (uint64_t*)PhysicalToKernelPtr(AllocatePages(1));
            for(int i = 0; i < 512; i++)
                pml1[i] = 0;
            pml2[pml2Index] = PML_SET_ADDR((uint64_t)KernelToPhysicalPtr(pml1)) | PML_SET_P(1) | PML_SET_RW(1);
        } else {
            pml1 = (uint64_t*)PhysicalToKernelPtr((void*)PML_GET_ADDR(pml2Entry));
        }

        pml1[pml1Index] = PML_SET_ADDR((uint64_t)phys) | PML_SET_P(1) | PML_SET_RW(1);

        __asm__ __volatile__ (
            "invlpg (%0)"
            : : "r"(virt)
        );
    }
	
    void UnmapKernelPage(void* virt)
    {
        uint64_t pml4Index = GET_PML4_INDEX((uint64_t)virt);
        uint64_t pml3Index = GET_PML3_INDEX((uint64_t)virt);
        uint64_t pml2Index = GET_PML2_INDEX((uint64_t)virt);
        uint64_t pml1Index = GET_PML1_INDEX((uint64_t)virt);

        uint64_t pml4Entry = pageTableLevel4[pml4Index];
        uint64_t* pml3 = (uint64_t*)PhysicalToKernelPtr((void*)PML_GET_ADDR(pml4Entry));

        uint64_t pml3Entry = pml3[pml3Index];
        uint64_t* pml2 = (uint64_t*)PhysicalToKernelPtr((void*)PML_GET_ADDR(pml3Entry));

        uint64_t pml2Entry = pml2[pml2Index];
        uint64_t* pml1 = (uint64_t*)PhysicalToKernelPtr((void*)PML_GET_ADDR(pml2Entry));

        pml1[pml1Index] = 0;

        __asm__ __volatile__ (
            "invlpg (%0)"
            : : "r"(virt)
        );
    }
}