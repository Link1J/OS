#include "Device.hpp"
#include "VFS.hpp"
#include "printf.h"
#include <string.h>

class PartitionData : public Device
{
    char* deviceName;
    char* partitionName;
    uint64_t LBAStart, LBAEnd;
public:
    PartitionData(const char* name, const char* deviceNameIn, char* data)
        : Device("/System/Drives", name)
    {
		uint64_t length = strlen(deviceNameIn);
    	deviceName = new char[length + 1];
		partitionName = new char[72];
		memcpy(deviceName, deviceNameIn, length + 1);
		memcpy(partitionName, data + 0x38, 72);

		LBAStart = *(uint64_t*)(data + 0x20);
		LBAEnd   = *(uint64_t*)(data + 0x28);

		for (int a = 0; a < 72; a += 2)
			partitionName[a / 2] = partitionName[a];
    }
                
    uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize)
	{
		char tempBuffer[200];

		int size = snprintf(tempBuffer, 200, 
		"Device: %s, Name: %s, LBA: %d,%d", 
		deviceName, partitionName, LBAStart, LBAEnd);
		
		size = (size - pos < bufferSize) ? size - pos : bufferSize;
		memcpy(buffer, tempBuffer + pos, size);
			
		int stringLen = strlen((char*)buffer); 
        return stringLen < size ? stringLen : size;
	}

    void Write(uint64_t pos, void* buffer, uint64_t bufferSize)
	{

	}
};

void CreatePartionsFromGPT(const char* driveName);

void CreatePartitions(const char* driveName)
{
    CreatePartionsFromGPT(driveName);
}

void CreatePartionsFromGPT(const char* driveName)
{
    char* buffer = new char[513];
    snprintf(buffer, 513, "/System/Drives/%s", driveName);
    uint64_t file = VFS::OpenFile(buffer);

    buffer[0] = 0xFF;
    buffer[1] = 1;
    VFS::ReadFile(file, buffer, 513);
    uint64_t size = *(uint32_t*)buffer;
    uint64_t a = 0, number = 0, id = 0;
    uint32_t numberOfPartitions;
    uint32_t sizeOfPartitionData;
    
    uint64_t length = strlen(driveName);
    char* fileName = new char[length + 5];

    printf("Drive Size: %d\n", size);

    if (size > 0)
    {
        do
        {
            buffer[0] = 1;
            number = VFS::ReadFile(file, buffer, 513);
            if (number > 0)
            {
                if (a == 1)
                {
                    if (*(uint64_t*)(buffer + 1) == 0x5452415020494645ULL)
                    {
                        numberOfPartitions = *(uint32_t*)(buffer + 0x51);
                        sizeOfPartitionData = *(uint32_t*)(buffer + 0x55);
                    }
                }
                else if (a >= 2 && a <= 33)
                {
                    char* buffer2 = new char[sizeOfPartitionData];
                    
                    for (int b = 0; b < number; b += sizeOfPartitionData)
                    {
                        char zeros[16] = {0};

                        memcpy(buffer2, buffer + 1 + b, sizeOfPartitionData);

                        if (memcmp(buffer2, zeros, 16) != 0)
                        {
                            snprintf(fileName, length + 5, "%sp%d", driveName, id);
                            new PartitionData(fileName, driveName, buffer2);
                            id++;
                        }
                    }

                    delete[] buffer2;
                }
            }
            a++;
            if (a > 33)
                number = 0;
        }
        while (number > 0);
    }

    delete[] fileName;
    delete[] buffer;
}