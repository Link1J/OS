#include "ACPI.hpp"
#include "ACPI_Structs.hpp"
#include "MemoryUtils.hpp"
#include "MemoryManager.hpp"
#include "Error.hpp"
#include "Terminal.hpp"

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
			if (!memcmp(header->Signature, tableId, 4))
				return (void *)header;
		}
 
		return nullptr;
	}	
	
	static void* RSDP;
	
	void Init(void* RSDPStructure)
	{		
		printf("Seting up APIC\n");
		
		RSDP = RSDPStructure;
		
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
		uint8_t* data = (uint8_t*)&dsdt->data;
		int printName = 0;
		

		/*for (uint8_t* inst = data; inst < data + size; inst++)
		{	
			switch (*inst)
			{
			case 0x5B:
				inst++;
				switch (*inst)
				{
				case 0x30:
					printf("\nRevision: ");
					break;
				case 0x82:
					printf("\nDevice: ");
					printName++;
					break;
				case 0x83:
					printf("\nProcessor: ");
					printName++;
					break;
				case 0x84:
					printf("\nPowerResource: ");
					printName++;
					break;
				}				
				break;
			case 0x10:
				printf("\nScope: ");
				printName++;
				break;
			case 0x12:
				printf("\nPackage: ");
				break;
			case 0x14:
				printf("\nMethod: ");
				printName++;
				break;
			case 0x06:
				//printf("\nAlias: ");
				break;
			case 0x08:
				printf("\nName: ");
				printName++;
				break;
			case '.':
				printName++;
				break;
			}
			
			if (printName > 0 && ((*inst >= 'A' && *inst <= 'Z') || *inst == '_'))
			{
				printName--;
				for (int a = 0; a < 4; a++)
				{
					if ((*inst >= 'A' && *inst <= 'Z') || (*inst >= '0' && *inst <= '9') || *inst == '_')
						Terminal::PrintChar(*inst);
					inst++;
				}
				Terminal::PrintChar(' ');
			}
		}*/
	}
}