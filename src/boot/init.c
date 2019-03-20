#include "init.h"
#include "util.h"

void initBootloader()
{	
	printf("Initializing bootloader...\n");
	
	EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_STATUS err = BS->HandleProtocol(IH, &guid, (void**)&LI);
    if(err != EFI_SUCCESS) {
        printf("Failed to init loaded image protocol\nPress any key to exit...\n");
		waitForKey();
        exit(EFI_LOAD_ERROR);
    }
	
	// Get Filesystem access of the device the bootloader is located on
    guid = (EFI_GUID)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    err = BS->HandleProtocol(LI->DeviceHandle, &guid, (void**)&FS);
    if(err != EFI_SUCCESS) {
        printf("Failed to init filesystem protocol\nPress any key to exit...\n");
        waitForKey();
        exit(EFI_LOAD_ERROR);
    }

    // Get Graphics Protocol of the screen
    guid = (EFI_GUID)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    err = BS->LocateProtocol(&guid, NULL, (void**)&G);
    if(err != EFI_SUCCESS) {
        printf("Failed to init grpahics protocol\nPress any key to exit...\n");
        waitForKey();
        exit(EFI_LOAD_ERROR);
    }
}