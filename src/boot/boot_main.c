#include "util.h"
#include "init.h"
#include "graphics.h"
#include "file.h"
#include "ELF.h"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
 
    /* Store the system table for future use in other functions */
    ST = SystemTable;
	BS = SystemTable->BootServices;
	IH = ImageHandle;
	
	initBootloader();
	setVideoMode();
	
	FILE_DATA data = openFile(L"\\kernel.elf");
	
	printf("%d\n", data.size);
	
    EFI_STATUS err = BS->ExitBootServices(LI, (void*)NULL);
    if(err != EFI_SUCCESS) {
		printf("Failed to exit boot services\nPress any key to exit...\n");
		waitForKey();
        return EFI_LOAD_ERROR;
    }
 
    asm("hlt");
}