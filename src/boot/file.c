#include "file.h"
#include "util.h"
#include "memory.h"

FILE_DATA openFile(const wchar_t* path)
{
	EFI_FILE_HANDLE volumeRoot;
	EFI_FILE_HANDLE file;
	FILE_DATA data;
	
	data.size = 0;
	data.data = NULL;
	
	FS->OpenVolume(FS, &volumeRoot);
	EFI_STATUS err = volumeRoot->Open(volumeRoot, &file, (CHAR16*)path, EFI_FILE_MODE_READ, 0);
	if(err != EFI_SUCCESS)
		return data;
	
	UINTN infoSize = 0;
	EFI_FILE_INFO* info;

	EFI_GUID guid = EFI_FILE_INFO_ID;
	// Get the size needed to store the file info
	err = file->GetInfo(file, &guid, &infoSize, NULL);
	if(err != EFI_BUFFER_TOO_SMALL)
		return data;
		
	// actually get the file info
	info = (EFI_FILE_INFO*)allocate(infoSize, EfiLoaderData);
	err = file->GetInfo(file, &guid, &infoSize, info);
	if(err != EFI_SUCCESS)
		return data;

	uint64_t size = info->FileSize;
	release(info, infoSize);

	uint8_t* buffer = (uint8_t*)allocate(size, (EFI_MEMORY_TYPE)0x80000001);

	err = file->Read(file, &size, buffer);
	if(err != EFI_SUCCESS) {
		release(buffer, size);
		return data;
	}

	data.size = size;
	data.data = buffer;
	
	return data;
}