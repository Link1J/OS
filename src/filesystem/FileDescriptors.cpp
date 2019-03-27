#include "FileDescriptors.hpp"
#include "MemoryUtils.hpp"
#include "printf.h"
#include "Error.hpp"

#include <string.h>

namespace VFS
{
	FileDescriptors::FileDescriptor* FileDescriptors::GetDescriptor(uint64_t id) {
        return descriptors[id - 1];
    }

    FileDescriptors::FileDescriptor* FileDescriptors::GetFreeDescriptor()
    {
        if(nextID != 0) {
            FileDescriptor* ret = GetDescriptor(nextID);
            nextID = ret->node;
            return ret;
        } else {
            FileDescriptor* node = new FileDescriptor();
            memset(node, 0, sizeof(FileDescriptor));
            node->id = descriptors.Size() + 1;
            descriptors.PushBack(node);
            return node;
        }
    }
	
	void FileDescriptors::Mount(Node& mountPoint)
	{
		descriptors = Vector<FileDescriptor*>();
		nextID = 0;
	}

	void FileDescriptors::Unmount() 
	{
		Error::Panic("Tried to Unmount the File Descriptors. Are you an idiot!");
	}

	void FileDescriptors::CreateNode(Node& folder, Node& node)
	{
		FileDescriptor* desc = GetFreeDescriptor();
		desc->pos = 0;
		
		node.type = Node::Type::FileDescriptor;
		node.file.size = sizeof(FileDescriptor);
		snprintf(node.name, 50, "%d", desc->id);		
		node.fileSystemNode = desc->id;
	}

	void FileDescriptors::DestroyNode(Node& folder, Node& node)
	{
		FileDescriptor* desc = GetDescriptor(node.fileSystemNode);
		desc->node = nextID;
		nextID = desc->id;
	}

	uint64_t FileDescriptors::ReadFile(const Node& node, uint64_t pos, void* buffer, uint64_t bufferSize)
	{
		if (buffer != nullptr)
			return 0;
		
		if ((int64_t)bufferSize == -1)
			return GetDescriptor(node.fileSystemNode)->pos;
		if ((int64_t)bufferSize == -2)
			return GetDescriptor(node.fileSystemNode)->node;
		if ((int64_t)bufferSize == -3)
			return GetDescriptor(node.fileSystemNode)->id;
		
		return 0;
	}

	uint64_t FileDescriptors::WriteFile(Node& node, uint64_t pos, void* buffer, uint64_t bufferSize)
	{
		if (buffer != nullptr)
			return 0;
		
		if ((int64_t)bufferSize == -1)
			GetDescriptor(node.fileSystemNode)->pos = pos;
		if ((int64_t)bufferSize == -2)
			GetDescriptor(node.fileSystemNode)->node = pos;
		
		return 0;
	}
}