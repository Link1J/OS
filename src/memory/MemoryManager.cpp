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
	struct Segment
	{
		Segment* next;
		Segment* prev;
		uint64_t base;
		uint64_t size
	};

	static Segment<T>* 	head;
	static Segment<T>* 	tail;
	static int 			size;
	
	static uint64_t		highMemoryBase;
	static uint64_t*	pageTableLevel4;
	
	void AddNode(void* memory, uint64_t size)
	{
		Segment* newNode = (Segment*)memory;
			
		newNode->base = (uint64_t)memory;
		newNode->base = size;
			
		newNode->next = nullptr;
		newNode->prev = nullptr;
			
		if (head == nullptr)
		{
			head = newNode;
			tail = newNode;
		}
		else
		{
			tail->next = newNode;
			newNode->prev = tail;
			tail = newNode;
		}
			
		size++;
	}
	
	void RemoveNode(Segment* newNode)
	{
		newNode->prev->next = newNode->next;
		
		if (tail == newNode)
			tail = newNode->prev;
		else 
			newNode->next->prev = newNode->prev;
			
		size--;
	}
		
	void Init(PhysicalMapSegment* physMapStart, uint64_t* pageBuffer, uint64_t highMemoryBaseIn)
	{
		printf("Starting Memory manager\n");
		
		head = nullptr;
		tail = nullptr;
		size = 0;
		
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
			
			AddNode(curr, size);
			
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
		if (size == 0)
			Error::Panic("Out of memory!");
		
		int sizeWanted = numPages * 4096;
		for (auto pages = head; pages != nullptr; pages = pages->next)
		{
			if (pages->data.size >= sizeWanted)
			{
				if (pages->data.size == sizeWanted)
				{
					RemoveNode(pages);
				}
				else if (pages->data.size > sizeWanted)
				{
					Segment* newNode = (Segment*)((char*)pages + sizeWanted);
				
					newNode->next = pages->next;
					newNode->prev = pages->prev;
					newNode->data.base = pages->data.base + sizeWanted;
					newNode->data.size = pages->data.size - sizeWanted;
					
					if(pages->prev != nullptr)
						pages->prev->next = newNode;
					else
						head = newNode;
					
					if(pages->next != nullptr)
						pages->next->prev = newNode;
					else
						tail = newNode;
				}
				return KernelToPhysicalPtr(pages);
			}
		}

        return nullptr;
    }

    void FreePages(void* pages, uint64_t numPages)
    {
		int size = numPages * 4096;		
		AddNode(pages, size);
    }
}