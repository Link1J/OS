#include "Device.hpp"
#include "Vector.hpp"
#include "VFS.hpp"
#include "printf.h"
#include <string.h>

static Vector<Device*> devices;

Device* Device::GetByID(uint64_t id)
{
    return devices[id];
}

Device::Device(const char* path, const char* name)
{
    id = devices.Size();
    devices.PushBack(this);

    int size = strlen(path);
    if (size != 0)
        size++;
    char* buffer = new char[10 + size];
    if (strlen(path) == 0)
    {
        memcpy(buffer, "/Devices", 10);
    }
    else
    {
        memcpy(buffer, "/Devices/", 10);
        memcpy(buffer + 9, path, size);
    }

    if(!VFS::CreateDeviceFile(buffer, name, id))
        printf("Failed to create device file for %s at path %s\n", name, buffer);
    else
        printf("Created device file for %s at path %s\n", name, buffer);
}