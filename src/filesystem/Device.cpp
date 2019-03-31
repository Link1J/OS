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
    char* buffer;

    /*if (*path != '/')
    {
        if (size != 0)
            size++;
        buffer = new char[9 + size];
        if (strlen(path) == 0)
        {
            memcpy(buffer, "/System", 9);
        }
        else
        {
            memcpy(buffer, "/System/", 9);
            memcpy(buffer + 8, path, size);
        }
    }
    else
    {*/
        buffer = (char*)path;
    //}

    if(!VFS::CreateDeviceFile(buffer, name, id))
        printf("Failed to create device file for %s at path %s\n", name, buffer);
    else
        printf("Created device file for %s at path %s\n", name, buffer);

    /*int file = VFS::OpenFolder(buffer);
    if (file != 0)
    {
        printf("%s still Good\n", buffer);
        VFS::CloseFile(file);
    }
    else
    {
        printf("%s is not Good\n", buffer);
    }*/
}