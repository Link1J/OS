#include "VFS.hpp"
#include "Vector.hpp"
#include "Stack.hpp"

namespace VFS
{
	class FileDescriptors : public FileSystem
	{
	public:
		struct FileDescriptor
		{
			uint64_t id;
			
			uint64_t node;
			uint64_t pos;
		};

	private:
		Vector<FileDescriptor*> descriptors;
		uint64_t nextID;
	
		FileDescriptor* GetDescriptor(uint64_t id);
		FileDescriptor* GetFreeDescriptor();
	
	public:
		void Mount		(Node& mountPoint	);
		void Unmount	(					);
	
		void CreateNode		(Node& folder, Node& node);
		void DestroyNode	(Node& folder, Node& node);
	
		uint64_t ReadFile	(const Node& node, uint64_t pos, void* buffer, uint64_t bufferSize);
		uint64_t WriteFile	(Node& node      , uint64_t pos, void* buffer, uint64_t bufferSize);
	};
}