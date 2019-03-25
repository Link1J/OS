#include "VFS.hpp"

class FileDecriptors : public FileSystem
{
	void Mount		(Node& mountPoint	);
	void Unmount	(					);

	void CreateNode		(Node& folder, Node& node);
	void DestroyNode	(Node& folder, Node& node);

	uint64_t ReadFile	(const Node& node, uint64_t pos, void* buffer, uint64_t bufferSize);
	void 	 WriteFile	(Node& node      , uint64_t pos, void* buffer, uint64_t bufferSize);
    
	void ReadDirEntries(Node& folder);
}