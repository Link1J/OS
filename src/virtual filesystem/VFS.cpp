#include "VFS.hpp"
#include "Vector.hpp"

namespace VFS
{
	struct FileDescriptor
    {
        uint64_t id;

        uint64_t node;
        uint64_t pos;
    };
	
	static Vector<Node*> nodes;
    static uint64_t firstFreeNode;
	
	Node* FindNode(Node* folder, const char* name)
	{
		if(folder->type != Node::TYPE_DIRECTORY)
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

        uint64 bufferPos = 0;

        while(true) {
            char c = path[pathPos];
            pathPos++;
            if(c == '/') {
                nameBuffer[bufferPos] = '\0';

                node = FindNode(node, nameBuffer);
                if(&node == nullptr)
                    return nullptr;
                if(node->type != Node::TYPE_DIRECTORY)
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

	
	void Init();

    void Mount(const char* mountPoint, FileSystem* fs)
	{
		
	}
    
    bool CreateFile(const char* folder, const char* name);
    bool CreateFolder(const char* folder, const char* name);
    bool CreateDeviceFile(const char* folder, const char* name, uint64_t devID);
    bool DeleteFile(const char* file);

    uint64_t OpenFile(const char* path);
    void CloseFile(uint64_t file);
    uint64_t GetFileSize(uint64_t file);

    uint64_t ReadFile(uint64_t file, void* buffer, uint64_t bufferSize);
    void WriteFile(uint64_t file, void* buffer, uint64_t bufferSize);
    void SeekFile(uint64_t file, uint64_t pos);

    Node* GetNode(uint64_t id) {
		if (
        return nodes[id - 1];
    }

    Node* GetFreeNode()
    {
        if(firstFreeNode != 0) {
            Node* ret = GetNode(firstFreeNode);
            g_FirstFreeNode = ret->numReaders;
            return ret;
        } else {
            Node* n = new Node();
            memset(n, 0, sizeof(Node));
            n->id = g_Nodes.size() + 1;
            g_Nodes.push_back(n);
            return n;
        }
    }
}