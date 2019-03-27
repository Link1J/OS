#include "VFS.hpp"

class FAT : public VFS::FileSystem
{
	struct FAT32ExtendedBootRecord
	{
		//extended fat32 stuff
		unsigned int		table_size_32;
		unsigned short		extended_flags;
		unsigned short		fat_version;
		unsigned int		root_cluster;
		unsigned short		fat_info;
		unsigned short		backup_BS_sector;
		unsigned char 		reserved_0[12];
		unsigned char		drive_number;
		unsigned char 		reserved_1;
		unsigned char		boot_signature;
		unsigned int 		volume_id;
		unsigned char		volume_label[11];
		unsigned char		fat_type_label[8];
	 
	}__attribute__((packed));
	 
	struct FAT16ExtendedBootRecord
	{
		//extended fat12 and fat16 stuff
		unsigned char		bios_drive_num;
		unsigned char		reserved1;
		unsigned char		boot_signature;
		unsigned int		volume_id;
		unsigned char		volume_label[11];
		unsigned char		fat_type_label[8];
	 
	}__attribute__((packed));
	 
	struct FATBootRecord
	{
		unsigned char 		bootjmp[3];
		unsigned char 		oem_name[8];
		unsigned short 	    bytes_per_sector;
		unsigned char		sectors_per_cluster;
		unsigned short		reserved_sector_count;
		unsigned char		table_count;
		unsigned short		root_entry_count;
		unsigned short		total_sectors_16;
		unsigned char		media_type;
		unsigned short		table_size_16;
		unsigned short		sectors_per_track;
		unsigned short		head_side_count;
		unsigned int 		hidden_sector_count;
		unsigned int 		total_sectors_32;
	 
		//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
		//unsigned char		extended_section[54];
		union {
			FAT16ExtendedBootRecord FAT16;
			FAT32ExtendedBootRecord FAT32;
		};
	}__attribute__((packed));
	
public:
	void Mount		(VFS::Node& mountPoint	);
	void Unmount	(						);

	void CreateNode		(VFS::Node& folder, VFS::Node& node);
	void DestroyNode	(VFS::Node& folder, VFS::Node& node);

	uint64_t ReadFile	(const VFS::Node& node, uint64_t pos, void* buffer, uint64_t bufferSize);
	uint64_t WriteFile	(VFS::Node& node      , uint64_t pos, void* buffer, uint64_t bufferSize);
};