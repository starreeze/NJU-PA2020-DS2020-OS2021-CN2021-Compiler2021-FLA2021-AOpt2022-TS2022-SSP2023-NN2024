#ifndef __DATA_H__
#define __DATA_H__

#include "types.h"

/*
*                                                        Layout of block group 0
*+--------------------+--------------------+-------------------+--------------------+--------------------+--------------------+
*|     SuperBlock     |   GroupDescTable   |    InodeBitmap    |    BlockBitmap     |     InodeTable     |     DataBlocks     |
*+--------------------+--------------------+-------------------+--------------------+--------------------+--------------------+
*      1024 Bytes           Many Block            1 Block             1 Block             Many Blocks        Many More Blocks
*
*                                                        Layout of block group 1
*+--------------------+--------------------+-------------------+--------------------+--------------------+--------------------+
*|     SuperBlock     |   GroupDescTable   |    InodeBitmap    |    BlockBitmap     |     InodeTable     |     DataBlocks     |
*+--------------------+--------------------+-------------------+--------------------+--------------------+--------------------+
*      1024 Bytes           Many Block            1 Block             1 Block             Many Blocks        Many More Blocks

*/

/*
* GROUP_SIZE is at least 1024 bytes + 3 blocks
* s: SECTOR_NUM
* b: SECTORS_PER_BLOCK
* g: GROUP_NUM
* g = \lfloor\frac{s}{2+\lceil(32g/512/b)\rceil*b+b+b+\lceil(128*512*b/512/b)\rceil*b+512*b*b}\rfloor
*/

/* Independent Variables */
//#define SECTOR_NUM 1024 // i.e., 512 KB, as we take file to simulate hard disk, SECTOR_NUM is bounded by (2^32 / 2^9) sectors for 32-bits system
#define SECTOR_NUM 8196
#define SECTOR_SIZE 512 // i.e., 512 B
#define SECTORS_PER_BLOCK 2
#define POINTER_NUM 12
#define NAME_LENGTH 64

/* Dependent Variables & Constants */
#define BLOCK_SIZE (SECTOR_SIZE * SECTORS_PER_BLOCK)

#define MAX_GROUP_NUM (SECTOR_NUM / SECTOR_SIZE / SECTORS_PER_BLOCK / 8 / SECTORS_PER_BLOCK + 1)

#define SUPER_BLOCK_SIZE 1024
#define GROUP_DESC_SIZE 32
#define INODE_BITMAP_SIZE BLOCK_SIZE
#define BLOCK_BITMAP_SIZE BLOCK_SIZE
#define INODE_SIZE 128
#define DIRENTRY_SIZE 128

#define UNKNOWN_TYPE 0
#define REGULAR_TYPE 1
#define DIRECTORY_TYPE 2
#define CHARACTER_TYPE 3
#define BLOCK_TYPE 4
#define FIFO_TYPE 5
#define SOCKET_TYPE 6
#define SYMBOLIC_TYPE 7

union SuperBlock {
	uint8_t byte[SUPER_BLOCK_SIZE];
	struct {
		int32_t sectorNum; // total number of sectors
		int32_t inodeNum; // total number of inodes
		int32_t blockNum; // total number of data blocks
		int32_t availInodeNum; // total number of available inodes
		int32_t availBlockNum; // total number of available data blocks
		int32_t blockSize; // number of bytes in each block
		int32_t inodesPerGroup; // number of inodes in each group
		int32_t blocksPerGroup; // number of blocks in each group
	};
};

typedef union SuperBlock SuperBlock;

union GroupDesc {
	uint8_t byte[GROUP_DESC_SIZE];
	struct {
		int32_t inodeBitmap; // sector as unit
		int32_t blockBitmap; // sector as unit
		int32_t inodeTable;  // sector as unit
		int32_t availInodeNum;
		int32_t availBlockNum;
	};
};

typedef union GroupDesc GroupDesc;

struct InodeBitmap {
	uint8_t byte[INODE_BITMAP_SIZE];
};

typedef struct InodeBitmap InodeBitmap;

struct BlockBitmap {
	uint8_t byte[BLOCK_BITMAP_SIZE];
};

typedef struct BlockBitmap BlockBitmap;

union Inode {
	uint8_t byte[INODE_SIZE];
	struct {
		int16_t type;  // further implement privilege control, i.e., drwxrwxrwx, uid, gid, others
		int16_t linkCount;
		int32_t blockCount;
		int32_t size;  // size of this file, byte as unit
		int32_t pointer[POINTER_NUM];
		int32_t singlyPointer;
		int32_t doublyPointer;
		int32_t triplyPointer;
	};
};

typedef union Inode Inode;

union DirEntry {
	uint8_t byte[DIRENTRY_SIZE];
	struct {
		int32_t inode; // byte as unit? TODO inode index as unit
		//int32_t size;  // size of this file, byte as unit
		//int8_t type; // may not be necessary
		char name[NAME_LENGTH];
	};
};

typedef union DirEntry DirEntry;

#endif
