namespace VFS
{
	struct Node
	{
		enum class Type {
			File,
            Directory,
            Device,
        } type;

        union {
            struct {
                uint64_t size;
            } file;
            struct {
                uint64_t numFiles;
                uint64_t* files;
            } directory;
            struct {
                uint64_t devID;
            } device;
        };

        char name[50];

        FileSystem* fs;
        uint64_t fsNode;

        uint64_t id;
        uint64_t numReaders;
        uint64_t numWriters;
    };

    class FileSystem
    {
    public:
        virtual void Mount		(Node& mountPoint	) = 0;
        virtual void Unmount	(					) = 0;

        virtual void CreateNode		(Node& folder, Node& node) = 0;
        virtual void DestroyNode	(Node& folder, Node& node) = 0;

        virtual uint64_t ReadFile	(const Node& node, uint64_t pos, void* buffer, uint64_t bufferSize) = 0;
        virtual void 	 WriteFile	(Node& node      , uint64_t pos, void* buffer, uint64_t bufferSize) = 0;
    
        virtual void ReadDirEntries(Node& folder) = 0;
    };

	void Init();

    void Mount(const char* mountPoint, FileSystem* fs);
    
    bool CreateFile			(const char* folder, const char* name					);
    bool CreateFolder		(const char* folder, const char* name					);
    bool CreateDeviceFile	(const char* folder, const char* name, uint64_t devID	);
    bool DeleteFile			(const char* file										);

    uint64_t	OpenFile	(const char* path	);
    void 		CloseFile	(uint64_t file		);
    uint64_t	GetFileSize	(uint64_t file		);

    uint64_t	ReadFile	(uint64_t file, void* buffer, uint64_t bufferSize	);
    void 		WriteFile	(uint64_t file, void* buffer, uint64_t bufferSize	);
    void		SeekFile	(uint64_t file, uint64_t pos						);

    Node* GetNode(uint64_t id);
    Node* GetFreeNode();

}