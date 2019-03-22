#include "util.h"
#include "init.h"
#include "file.h"
#include "ELF.h"
#include "memorymap.h"

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
	
    BS->SetWatchdogTimer(0, 0, 0, NULL);
	
	KernelHeader* header = NULL;
	initKernelHeader(&header);
	
	printf("Loading modules\n");
	
	// Load Kernel
	FILE_DATA kernelData = openFile(L"\\kernel.elf");
	if(kernelData.size == 0) {
		printf("Failed to load kernel image\nPress any key to exit\n");
        waitForKey();
        return EFI_LOAD_ERROR;
    }
	
	printf("Preparing kernel\n");
	
	uintptr_t	kernelEntryPoint = 0;
	uint64_t	size = getELFSize(kernelData.data);
    // allocate the buffer the relocated kernel image will be in
    uint8_t*	processBuffer = (uint8_t*)convertPtr(size, (EFI_MEMORY_TYPE)0x80000001);
	
	// relocate the kernel into the buffer
	if(!prepareELF(kernelData.data, processBuffer, &kernelEntryPoint)) {
        printf("Failed to prepare kernel\nPress any key to exit\n");
        waitForKey();
        return EFI_LOAD_ERROR;
    }
	
	if(kernelEntryPoint == 0) {
        printf("Kernel entry point not found\nPress any key to exit\n");
        waitForKey();
        return EFI_LOAD_ERROR;
    }
	
	// the old buffer is not needed anymore
    release(kernelData.data, kernelData.size);

	header->kernelImage.buffer = processBuffer;
    header->kernelImage.numPages = (size + 4095) / 4096;
	
	getMemoryMap();
	closeBootServices();
	
	header->physMapStart = build();
    header->physMapEnd = header->physMapStart;
    while(header->physMapEnd->next != NULL)
        header->physMapEnd = header->physMapEnd->next;

	typedef void (*KernelMain)(KernelHeader* header);
    KernelMain kernelMain = (KernelMain)kernelEntryPoint;
    char* kernelStackTop = (char*)(header->stack) + header->stackPages * 4096;

    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "movq rbp, %2;"             
        "movq rsp, %2;"             // switch stack to kernel stack
        "callq rax;"                // call the kernel
        ".att_syntax prefix"
        : : "D"(header), "a"(kernelMain), "r"(kernelStackTop)
    );
	
    asm("hlt");
}