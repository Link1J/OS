#include "VFS.hpp"
#include "Vector.hpp"
#include "FileDescriptors.hpp"
#include "printf.h"
#include "Error.hpp"
#include "MemoryUtils.hpp"
#include "Device.hpp"

#include <string.h>

namespace VFS
{
	class NullFileSystem : public FileSystem
	{
	public:
		void Mount		(Node& mountPoint	) {}
		void Unmount	(					) {}
	
		void CreateNode		(Node& folder, Node& node) {}
		void DestroyNode	(Node& folder, Node& node) {}
	
		uint64_t ReadFile	(const Node& node, uint64_t pos, void* buffer, uint64_t bufferSize) { return 0; }
		uint64_t WriteFile	(Node& node      , uint64_t pos, void* buffer, uint64_t bufferSize) { return 0; }
		
		void ReadDirEntries(Node& folder) {}
	};
	
	static Vector<Node*> nodes;
    static uint64_t firstFreeNode;
	
	Node* FindNode(Node* folder, const char* name)
	{
		if(folder->type != Node::Type::Directory)
			return nullptr;

		for(int i = 0; i < folder->directory.numFiles; i++) {
			Node* node = GetNode(folder->directory.files[i]);
			if(strcmp(name, node->name) == 0)
				return node;
        }
		
		return nullptr;
    }

    Node* FindPath(const char* path)
    {
        static char nameBuffer[50] = { 0 };

        uint64_t pathPos = 1;
        Node* node = GetNode(1);

        if(path[pathPos] == '\0')
            return node;

        uint64_t bufferPos = 0;

        while(true) {
            char c = path[pathPos];
            pathPos++;
            if(c == '/') {
                nameBuffer[bufferPos] = '\0';

                node = FindNode(node, nameBuffer);
                if(node == nullptr)
                    return nullptr;
                if(node->type != Node::Type::Directory)
                    return nullptr;

                bufferPos = 0;
            } else if(c == '\0') {
                nameBuffer[bufferPos] = '\0';
                return FindNode(node, nameBuffer);
            } else {
                nameBuffer[bufferPos] = c;
                bufferPos++;
            }
        }
    }
	
	void Init()
	{
		printf("Setting up VFS\n");
		
		Node* root = new Node();
        root->id = 1;
		memcpy(root->name, "/", 2);
        root->type = Node::Type::Directory;
        root->directory.numFiles = 0;
        root->directory.files = nullptr;
		root->fileSystem = new NullFileSystem();
        root->numReaders = 0;
        root->numWriters = 0;
		root->fileSystem->Mount(*root);
        nodes.PushBack(root);
		
		firstFreeNode = 0;

		if(!CreateFolder("/", "FileDescriptors"))
			Error::Panic("Failed to create /FileDescriptors folder\n");
		Mount("/FileDescriptors", new FileDescriptors());
		
		if(!CreateFolder("/", "System"))
			Error::Panic("Failed to create /System folder\n");
		
		if(!CreateFolder("/", "Boot"))
			Error::Panic("Failed to create /Boot folder\n");
		
		printf("VFS setup completed\n");
	}

    void Mount(const char* mountPoint, FileSystem* fileSystem)
	{
		Node* folder = FindPath(mountPoint);
		if(folder == nullptr || folder->type != Node::Type::Directory)
			return;
		if(folder->directory.numFiles != 0)
			return;

		folder->fileSystem = fileSystem;
		folder->fileSystem->Mount(*folder);
	}
	
	Node* CreateNode(const char* folder, const char* name)
	{
		Node* folderNode = FindPath(folder);
        if(folderNode == nullptr)
            return nullptr;
		if (folderNode->type != Node::Type::Directory)
			return nullptr;

        Node* newNode = GetFreeNode();
		memcpy(newNode->name, name, strlen(name) + 1);
        newNode->fileSystem = folderNode->fileSystem;
        newNode->numReaders = 0;
        newNode->numWriters = 0;
        newNode->fileSystem->CreateNode(*folderNode, *newNode);

        uint64_t* files = new uint64_t[folderNode->directory.numFiles + 1];
        memcpy(files, folderNode->directory.files, folderNode->directory.numFiles * sizeof(uint64_t));
        files[folderNode->directory.numFiles] = newNode->id;
        delete[] folderNode->directory.files;
        folderNode->directory.files = files;
        folderNode->directory.numFiles++;
		
		return newNode;
	}
    
	uint64_t CreateDecriptor(Node* file)
	{
		Node* desc = CreateNode("/FileDescriptors", "");
		if(desc == nullptr)
            return 0;
		desc->fileSystem->WriteFile(*desc, file->id, nullptr, -2);
		return desc->fileSystem->ReadFile(*desc, 0, nullptr, -3);
	}
	
    bool CreateFile(const char* folder, const char* name)
	{
		Node* newNode = CreateNode(folder, name);
		
		if (newNode == nullptr)
			return false;
		
        newNode->type = Node::Type::File;
		newNode->file.size = 0;

        return true;
	}
	
    bool CreateFolder(const char* folder, const char* name)
	{
		Node* newNode = CreateNode(folder, name);
		
		if (newNode == nullptr)
			return false;
		
        newNode->type = Node::Type::Directory;
        newNode->directory.numFiles = 0;
        newNode->directory.files = nullptr;
        
        return true;
	}
	
    bool CreateDeviceFile(const char* folder, const char* name, uint64_t devID)
	{
		Node* newNode = CreateNode(folder, name);
		
		if (newNode == nullptr)
			return false;
		
        newNode->type = Node::Type::Device;
        newNode->device.devID = devID;
        
        return true;
	}
	
	bool DestroyNode(Node* folder, Node* node)
	{
		if (node->type == Node::Type::Directory)
		{
			for (int a = 0; a < node->directory.numFiles; a++)
				DestroyNode(node, GetNode(node->directory.files[a]));
			delete node->directory.files;
		}

		node->fileSystem->DestroyNode(*folder, *node);

		uint64_t* temp = new uint64_t[folder->directory.numFiles];
		int o = 0;
		for (int a = 0; a < folder->directory.numFiles; a++)
		{
			if (folder->directory.files[a] == node->id)
				o = 1;
			else
				temp[a - o] = folder->directory.files[a];
		}
		
		uint64_t* temp2 = new uint64_t[folder->directory.numFiles - o];
		for (int a = 0; a < folder->directory.numFiles - o; a++)
		{
			temp2[a] = temp[a];
		}

		delete folder->directory.files;
		delete temp;
		folder->directory.files = temp2;
		folder->directory.numFiles -= o;
		
		node->numReaders = firstFreeNode;
		firstFreeNode = node->id;
		
		return true;
	}
	
    bool DeleteFile(const char* file)
	{
		char* folder = new char[strlen(file)];
		memcpy(folder, file, strlen(file));
		for (int a = strlen(file) - 1; a >= 0; a++)
		{
			if (folder[a] == '/')
				break;
			folder[a] = '\0';
		}
		
		Node* node = FindPath(file);
		Node* folderNode = FindPath(folder);
		
		delete[] folder;
		
        if(node == nullptr)
            return false;
		
		return DestroyNode(folderNode, node);
	}

    uint64_t OpenFile(const char* path)
	{
		Node* node = FindPath(path);
        if(node == nullptr)
            return 0;
        return CreateDecriptor(node);
	}

	uint64_t OpenFolder(const char* path)
	{
		Node* node = FindPath(path);
        if(node == nullptr)
            return 0;
		if(node->type != Node::Type::Directory)
			return 0;
        return CreateDecriptor(node);
	}

	Node* GetFileDescriptor(uint64_t file)
	{
		char name[50 + 18];
		snprintf(name, 50 + 18, "/FileDescriptors/%d", file);
		Node* desc = FindPath(name);
		return desc;
	}
	
	Node* GetNodeFromFileDescriptor(Node* desc)
	{
		if(desc == nullptr)
			return 0;
		uint64_t id = desc->fileSystem->ReadFile(*desc, 0, nullptr, -2);
		if (id == 0)
			return nullptr;
		return GetNode(id);
	}
	
	void CloseFile(uint64_t file)
	{
		DestroyNode(FindPath("/FileDescriptors"), GetFileDescriptor(file));
	}
	
    uint64_t GetFileSize(uint64_t file)
	{
		Node* desc = GetFileDescriptor(file);
		Node* node = GetNodeFromFileDescriptor(desc);
		if(node == nullptr)
            return 0;
		
		if(node->type == Node::Type::File)
			return node->file.size;
		if(node->type == Node::Type::Directory)
			return node->directory.numFiles * 50;
		if(node->type == Node::Type::Device)
			return -1;
		
		return 0;
	}

    uint64_t ReadFile(uint64_t file, void* buffer, uint64_t bufferSize)
	{
		if (bufferSize == 0 || buffer == nullptr)
			return 0;
	
		Node* desc = GetFileDescriptor(file);
		Node* node = GetNodeFromFileDescriptor(desc);
		if(node == nullptr)
            return 0;
		
		uint64_t pos = desc->fileSystem->ReadFile(*desc, 0, nullptr, -1);
		uint64_t change = 0;
		
		if (node->type == Node::Type::Directory)
		{
			if (pos < node->directory.numFiles * 50)
			{				
				Node* file = GetNode(node->directory.files[pos / 50]);
				change = strlen(file->name) + 1 - (pos % 50);
				change = change > bufferSize ? bufferSize : change;
				memcpy(buffer, file->name + (pos % 50), change);
				if (((char*)buffer)[change - 1] == 0)
				{
					desc->fileSystem->WriteFile(*desc, ((pos + change + 50) / 50) * 50, nullptr, -1);
				}					
				else
				{
					desc->fileSystem->WriteFile(*desc, pos + change, nullptr, -1);
				}
			}
		}
		else if(node->type == Node::Type::Device) 
		{
			Device* dev = Device::GetByID(node->device.devID);
			change = dev->Read(pos, buffer, bufferSize);
			desc->fileSystem->WriteFile(*desc, pos + change, nullptr, -1);
		}
		else
		{
			change = node->fileSystem->ReadFile(*node, pos, buffer, bufferSize);
			desc->fileSystem->WriteFile(*desc, pos + change, nullptr, -1);
		}
		
		return change;
	}
	
    void WriteFile(uint64_t file, void* buffer, uint64_t bufferSize)
	{
		if (bufferSize == 0 || buffer == nullptr)
			return;
	
		Node* desc = GetFileDescriptor(file);
		Node* node = GetNodeFromFileDescriptor(desc);
		if(node == nullptr || node->type == Node::Type::Directory)
            return;
		
		if(node->type == Node::Type::Device) 
		{
			Device* dev = Device::GetByID(node->device.devID);
			dev->Write(0, buffer, bufferSize);
		}
		else
		{
			uint64_t pos = desc->fileSystem->ReadFile(*desc, 0, nullptr, -1);
			uint64_t change = node->fileSystem->WriteFile(*node, pos, buffer, bufferSize);
			desc->fileSystem->WriteFile(*desc, pos + change, nullptr, -1);
		}
	}

    Node::Type GetType(uint64_t file)
	{
		Node* desc = GetFileDescriptor(file);
		Node* node = GetNodeFromFileDescriptor(desc);
		if(node == nullptr)
            return Node::Type::File;

		return node->type;		
	}
	
    void SeekFile(uint64_t file, uint64_t pos)
	{	
		Node* node = GetFileDescriptor(file);
		if(node == nullptr || node->type == Node::Type::FileDescriptor)
            return;
		
		node->fileSystem->WriteFile(*node, pos, nullptr, -1);
	}

    Node* GetNode(uint64_t id) {
        return nodes[id - 1];
    }

    Node* GetFreeNode()
    {
        if(firstFreeNode != 0) {
            Node* ret = GetNode(firstFreeNode);
            firstFreeNode = ret->numReaders;
            return ret;
        } else {
            Node* node = new Node();
            memset(node, 0, sizeof(Node));
            node->id = nodes.Size() + 1;
            nodes.PushBack(node);
            return node;
        }
    }
}