#include "ACPI.hpp"
#include "ACPI_Structs.hpp"
#include "MemoryUtils.hpp"
#include "MemoryManager.hpp"
#include "Error.hpp"

#include <string.h>
#include "printf.h"

namespace ACPI
{
	void* FindTable(const char* tableId, XSDT* xsdtDesc)
	{
		int entries = (xsdtDesc->header.Length - sizeof(xsdtDesc->header)) / 8;
		uint64_t* table = (&xsdtDesc->PointerToOtherSDT);
		
		for (int i = 0; i < entries; i++)
		{
			ACPISDTHeader* header = (ACPISDTHeader*)MemoryManager::PhysicalToKernelPtr((void*)table[i]);
			printf("%d %d %p %p %p\n",entries, i, header, table, (void*)table[i]);
			printf("%4.4s %4.4s\n", header->Signature, tableId);			
			if (!memcmp(header->Signature, tableId, 4))
				return (void *)header;
		}
 
		return nullptr;
	}	
	
	void Init(void* RSDPStructure)
	{		
		printf("Seting up APIC\n");
		
		RSDPDescriptor* desc = (RSDPDescriptor*)RSDPStructure;
		
		printf("APIC OEM: %.6s\n", desc->OEMID);
		
		XSDT* xsdtDesc = (XSDT*)MemoryManager::PhysicalToKernelPtr((void*)desc->XsdtAddress);
		
		if (memcmp(xsdtDesc->header.Signature, "XSDT", 4) != 0)
		{
			Error::Panic("Invalid XSDT Table");
		}
		
		FADT* fadt = (FADT*)FindTable("FACP", xsdtDesc);
		
		DSDT* dsdt = (DSDT*)MemoryManager::PhysicalToKernelPtr((void*)fadt->X_Dsdt);
		
		int size = dsdt->header.Length - sizeof(dsdt->header);
		
		for (int a = 0; a < 0x32; a++)
		{
			printf("%02hhX ",  ((char*)&dsdt->data)[a]);
		}
	}
}