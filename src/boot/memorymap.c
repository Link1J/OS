#include "memorymap.h"
#include "util.h"

UINTN memoryMapSize;
EFI_MEMORY_DESCRIPTOR* memMap;
UINTN descSize;
UINT32 descVersion;
uint64_t counter;

const char* memory_types[] = 
{
    "EfiReservedMemoryType",
    "EfiLoaderCode",
    "EfiLoaderData",
    "EfiBootServicesCode",
    "EfiBootServicesData",
    "EfiRuntimeServicesCode",
    "EfiRuntimeServicesData",
    "EfiConventionalMemory",
    "EfiUnusableMemory",
    "EfiACPIReclaimMemory",
    "EfiACPIMemoryNVS",
    "EfiMemoryMappedIO",
    "EfiMemoryMappedIOPortSpace",
    "EfiPalCode",
};

const char* memory_sizes[] = 
{
    "B",
    "KB",
    "MB",
    "GB",
    "TB",
};

void getMemoryMap()
{
	memoryMapSize = 0;
	// Get Memory map size
	EFI_STATUS err = BS->GetMemoryMap(&memoryMapSize, memMap, &mapKey, &descSize, &descVersion);
	if(err != EFI_BUFFER_TOO_SMALL) {
		printf("Failed to get memory map size\n");
		waitForKey();
        return EFI_LOAD_ERROR;
	}
	// allocate a page more than needed for the memory map, as this allocation will change the memory map
	memMap = (EFI_MEMORY_DESCRIPTOR*)allocate(memoryMapSize + 4096, (EFI_MEMORY_TYPE)0x80000001);
	memoryMapSize += 4096;
	err = BS->GetMemoryMap(&memoryMapSize, memMap, &mapKey, &descSize, &descVersion);
	if(err != EFI_SUCCESS) {
		printf("Failed to query memory map\n");
		waitForKey();
        return EFI_LOAD_ERROR;
	}
	
}

void printMemoryInfo()
{	
	uint64_t total  	 	= 0;
	uint64_t totalUsable	= 0;
	uint64_t totalUnusable	= 0;	
	
	for(uint64_t s = 0; s < counter; s++)
	{
		EFI_MEMORY_DESCRIPTOR *desc =  (EFI_MEMORY_DESCRIPTOR *)((uint8_t*)memMap + s * descSize);
		printf("Map %d:\n\
		\tType         : %s\n\
		\tPhysicalStart: 0x%016X\n\
		\tSize         : 0x%016X\n\
		\tNumberOfPages: %d(4k)\n",
			s, memory_types[desc->Type], desc->PhysicalStart, desc->NumberOfPages * 4096,
			desc->NumberOfPages);
		//waitForKey();
		if (desc->Type == EfiConventionalMemory)
			totalUsable		+= desc->NumberOfPages * 4096;
		else
			totalUnusable	+= desc->NumberOfPages * 4096;
		total += desc->NumberOfPages * 4096;
	}
	
	uint8_t totalSize, totalUsableSize, totalUnusableSize;
	double temp = 0;
	
	for (totalSize = 0, temp = total; temp > 1024; totalSize++)
		temp /= 1024.;
	total = ceil(temp);
	
	for (totalUsableSize = 0, temp = totalUsable; temp > 1024; totalUsableSize++)
		temp /= 1024.;
	totalUsable = ceil(temp);
	
	for (totalUnusableSize = 0, temp = totalUnusable; temp > 1024; totalUnusableSize++)
		temp /= 1024.;
	totalUnusable = ceil(temp);
	
	
	printf("Total Memory: %d%s\n\
	\tUsable  : %d%s\n\
	\tUnusable: %d%s\n",
			total, memory_sizes[totalSize], 
			totalUsable, memory_sizes[totalUsableSize],
			totalUnusable, memory_sizes[totalUnusableSize]);
}

void cleanUpMemMap(uint8_t* memMap, uint64_t size, uint64_t descSize, uint32_t* counter, uint32_t* total)
{
	uint8_t* endOfMemoryMap	= memMap + size;
	uint8_t* offset 		= memMap;
	
	uint32_t temp1 = 0, temp2 = 0;
	
	if (counter == NULL)
		counter = &temp1;
	if (total == NULL)
		total = &temp2;

	// Simplfy sectors
	while (offset < endOfMemoryMap)
	{
		EFI_MEMORY_DESCRIPTOR *seg = (EFI_MEMORY_DESCRIPTOR *)offset;
		
		if(	seg->Type == EfiLoaderCode || 
			seg->Type == EfiLoaderData ||
			seg->Type == EfiBootServicesCode ||
			seg->Type == EfiBootServicesData ||
			seg->Type == EfiConventionalMemory)
		{
			seg->Type = EfiConventionalMemory;
		}
		else 
		{
			seg->Type = EfiUnusableMemory;
		}
		
		offset += descSize;
		(*counter)++;
		(*total)++;
	}
	
	// join adjacent usable segments
	for(uint64_t s = 0; s < *counter; s++)
	{
		EFI_MEMORY_DESCRIPTOR *seg = (EFI_MEMORY_DESCRIPTOR *)(memMap + s * descSize);
		
		if(seg->PhysicalStart == (uint64_t)-1)  // segment was joined with another segment
			continue;
		
		uint64_t start	= seg->PhysicalStart;
		uint64_t end	= seg->PhysicalStart + seg->NumberOfPages * 4096;
		
		// search for adjacent segments
		for(uint64_t c = s + 1; c < *counter; c++) 
		{
			EFI_MEMORY_DESCRIPTOR* cmp = (EFI_MEMORY_DESCRIPTOR*)(memMap + c * descSize);
			if(cmp->Type != seg->Type)
				continue;
			if(cmp->PhysicalStart == (uint64_t)-1)  // segment was joined with another segment
				continue;

			uint64_t cmpStart = cmp->PhysicalStart;
			uint64_t cmpEnd = cmp->PhysicalStart + cmp->NumberOfPages * 4096;

			// cmp after seg
			if(cmpStart == end) 
			{
				seg->NumberOfPages += cmp->NumberOfPages;
				cmp->PhysicalStart = (uint64_t)-1;
				s--; // search for further adjacent entries
				break;
			}  // cmp before seg 
			else if(cmpEnd == start) 
			{
				seg->PhysicalStart = cmp->PhysicalStart;
				seg->NumberOfPages += cmp->NumberOfPages;
				cmp->PhysicalStart = (uint64_t)-1;
				s--;
				break;
			}
		}
	}
	
	// remove unused memorymap entries
	for(uint64_t s = 0; s < *counter; s++) 
	{
		EFI_MEMORY_DESCRIPTOR* seg = (EFI_MEMORY_DESCRIPTOR*)(memMap + s * descSize);
		if(seg->PhysicalStart != (uint64_t)-1)
			continue;

		for(uint64_t m = s + 1; m < *counter; m++) 
		{
			EFI_MEMORY_DESCRIPTOR* dest = (EFI_MEMORY_DESCRIPTOR*)(memMap + (m - 1) * descSize);
			EFI_MEMORY_DESCRIPTOR* src = (EFI_MEMORY_DESCRIPTOR*)(memMap + m * descSize);
	
			*dest = *src;
		}

		(*counter)--;
		s--;
	}
}

PhysicalMapSegment* build()
{
	cleanUpMemMap((uint8_t*)memMap, memoryMapSize, descSize, &counter, NULL);
	
	uint64_t sizeNeeded = 0;
	uint64_t numSegments = 0;

	PhysicalMapSegment* firstSegment = NULL;
	PhysicalMapSegment* lastSegment = NULL;

	for(uint64_t i = 0; i < counter; i++) 
	{
		EFI_MEMORY_DESCRIPTOR* desc = &memMap[i];
            
		if(desc->Type == EfiConventionalMemory)
		{
			// Just store this structure in the physical page it represents
			PhysicalMapSegment* seg = (PhysicalMapSegment*)convertPtr((void*)desc->PhysicalStart);

			// Insert it into the list
			if(firstSegment == NULL) 
			{
				firstSegment = seg;
				lastSegment = firstSegment;

				firstSegment->base = desc->PhysicalStart;
				firstSegment->numPages = desc->NumberOfPages;
				firstSegment->prev = NULL;
				firstSegment->next = NULL;
			} 
			else 
			{
				lastSegment->next = seg;
				seg->prev = lastSegment;
				lastSegment = seg;
                    
				seg->base = desc->PhysicalStart;
				seg->numPages = desc->NumberOfPages;
				seg->next = NULL;
			}
		}
	}

	return firstSegment;
}
