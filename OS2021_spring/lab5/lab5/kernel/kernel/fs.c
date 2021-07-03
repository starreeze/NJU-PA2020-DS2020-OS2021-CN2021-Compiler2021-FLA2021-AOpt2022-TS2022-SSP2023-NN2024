#include "x86.h"
#include "device.h"
#include "fs.h"

int calGroupNum (int sectorNum, int sectorsPerBlock) {
	int groupDescBlocks = 1;
	int groupNum = 0;
	int quotient = 0;
	int remainder = 0;
	int groupHeaderSectors = 0;
	while (1) {
		quotient = sectorNum / (sizeof(SuperBlock) / SECTOR_SIZE +  // SuperBlock
			groupDescBlocks * sectorsPerBlock +               // GourpDescTable
			(1 + 1 + sizeof(Inode) * 8) * sectorsPerBlock +          // BlockBitmap, InodeBitmap, InodeTable
			SECTOR_SIZE * sectorsPerBlock * 8 * sectorsPerBlock); // DataBlocks
		remainder =  sectorNum % (sizeof(SuperBlock) / SECTOR_SIZE +
			groupDescBlocks * sectorsPerBlock +
			(1 + 1 + sizeof(Inode) * 8) * sectorsPerBlock +
			SECTOR_SIZE * sectorsPerBlock * 8 * sectorsPerBlock);
		groupHeaderSectors = sizeof(SuperBlock) / SECTOR_SIZE + (groupDescBlocks + 1 + 1) * sectorsPerBlock; // SuperBlock + GroupDescTable + BlockBitmap + InodeBitmap
		groupNum = SECTOR_SIZE / sizeof(GroupDesc) * groupDescBlocks * sectorsPerBlock;
		if (quotient == 0 && remainder < groupHeaderSectors) {
			return 0;
		}
		else if (quotient == 0 && remainder >= groupHeaderSectors) {
			return 1;
		}
		else if (quotient <= groupNum && remainder < groupHeaderSectors) {
			return quotient;
		}
		else if (quotient < groupNum && remainder >= groupHeaderSectors) {
			return quotient + 1;
		}
		else if (quotient == groupNum && remainder >= groupHeaderSectors) {
			groupDescBlocks = groupDescBlocks + 1;
		}
		else if (quotient > groupNum) {
			groupDescBlocks = groupDescBlocks + 1;
		}
	}
}

int calGroupSize (int sectorNum, int sectorsPerBlock, int groupNum, int index) { // sector as unit
	int groupDescBlocks = (groupNum * sizeof(GroupDesc) + SECTOR_SIZE * sectorsPerBlock - 1) / (SECTOR_SIZE * sectorsPerBlock);
	int groupHeaderSectors = sizeof(SuperBlock) / SECTOR_SIZE + (groupDescBlocks + 1 + 1) * sectorsPerBlock; // SuperBlock + GroupDescTable + BlockBitmap + InodeBitmap
	int groupBodySectors = sizeof(Inode) * 8 * sectorsPerBlock + SECTOR_SIZE * sectorsPerBlock * 8 * sectorsPerBlock; // InodeTable + Data Blocks
	int remainder = sectorNum % (groupHeaderSectors + groupBodySectors);
	if (index < 0)
		return 0;
	else if (index + 1 < groupNum)  // before last group
		return groupHeaderSectors + groupBodySectors;
	else if (index + 1 == groupNum) { // last group
		if (remainder >= groupHeaderSectors)
			return remainder;
		else
			return groupHeaderSectors + groupBodySectors;
	}
	else
		return 0;
}

int calInodesPerGroup (int sectorNum, int sectorsPerBlock, int groupNum, int index) {
	int groupDescBlocks = (groupNum * sizeof(GroupDesc) + SECTOR_SIZE * sectorsPerBlock - 1) / (SECTOR_SIZE * sectorsPerBlock);
	int groupHeaderSectors = sizeof(SuperBlock) / SECTOR_SIZE + (groupDescBlocks + 1 + 1) * sectorsPerBlock; // SuperBlock + GroupDescTable + BlockBitmap + InodeBitmap
	int groupBodySectors = sizeof(Inode) * 8 * sectorsPerBlock + SECTOR_SIZE * sectorsPerBlock * 8 * sectorsPerBlock; // InodeTable + Data Blocks
	int remainder = sectorNum % (groupHeaderSectors + groupBodySectors);
	if (index < 0)
		return 0;
	else if (index + 1 < groupNum) // before last group
		return SECTOR_SIZE * sectorsPerBlock * 8;
	else if (index + 1 == groupNum) { // last group
		if (remainder >= groupHeaderSectors) {
			if (remainder - groupHeaderSectors >= sizeof(Inode) * 8 * sectorsPerBlock)
				return SECTOR_SIZE * sectorsPerBlock * 8;
			else
				return (remainder - groupHeaderSectors) / sectorsPerBlock * sectorsPerBlock * SECTOR_SIZE / sizeof(Inode);
		}
		else
			return SECTOR_SIZE * sectorsPerBlock * 8;
	}
	else
		return 0;
}

int calBlocksPerGroup (int sectorNum, int sectorsPerBlock, int groupNum, int index) {
	int groupDescBlocks = (groupNum * sizeof(GroupDesc) + SECTOR_SIZE * sectorsPerBlock - 1) / (SECTOR_SIZE * sectorsPerBlock);
	int groupHeaderSectors = sizeof(SuperBlock) / SECTOR_SIZE + (groupDescBlocks + 1 + 1) * sectorsPerBlock; // SuperBlock + GroupDescTable + BlockBitmap + InodeBitmap
	int groupBodySectors = sizeof(Inode) * 8 * sectorsPerBlock + SECTOR_SIZE * sectorsPerBlock * 8 * sectorsPerBlock; // InodeTable + Data Blocks
	int remainder = sectorNum % (groupHeaderSectors + groupBodySectors);
	if (index < 0)
		return 0;
	else if (index + 1 < groupNum) // before last group
		return SECTOR_SIZE * sectorsPerBlock * 8;
	else if (index + 1 == groupNum) { // last group
		if (remainder >= groupHeaderSectors) {
			if (remainder - groupHeaderSectors >= sizeof(Inode) * 8 * sectorsPerBlock)
				return (remainder - groupHeaderSectors - sizeof(Inode) * 8 * sectorsPerBlock) / sectorsPerBlock;
			else
				return 0;
		}
		else
			return SECTOR_SIZE * sectorsPerBlock * 8;
	}
	else
		return 0;
}

//int initGroupHeader (FILE *file, int sectorNum, int sectorsPerBlock, SuperBlock *superBlock, GroupDesc *groupDesc) {
int initGroupHeader (int sectorNum, int sectorsPerBlock, SuperBlock *superBlock, GroupDesc *groupDesc) {
	int i;
	int groupNum = calGroupNum(sectorNum, sectorsPerBlock);
	if (groupNum == 0)
		return -1;
	int inodeNum = 0;
	int blockNum = 0;
	int inodesPerGroup = 0;
	int blocksPerGroup = 0;
	int groupDescBlocks = (groupNum * sizeof(GroupDesc) + SECTOR_SIZE * sectorsPerBlock - 1) / (SECTOR_SIZE * sectorsPerBlock);
	int inodeBitmapOffset = sizeof(SuperBlock) / SECTOR_SIZE + groupDescBlocks * sectorsPerBlock;
	int blockBitmapOffset = sizeof(SuperBlock) / SECTOR_SIZE + (groupDescBlocks + 1) * sectorsPerBlock;
	int inodeTableOffset = sizeof(SuperBlock) / SECTOR_SIZE + (groupDescBlocks + 1 + 1) * sectorsPerBlock;
	int groupSize = calGroupSize(sectorNum, sectorsPerBlock, groupNum, 0);
	int counter = 0;
	for (i = 0; i < groupNum; i ++) {
		inodesPerGroup = calInodesPerGroup(sectorNum, sectorsPerBlock, groupNum, i);
		blocksPerGroup = calBlocksPerGroup(sectorNum, sectorsPerBlock, groupNum, i);
		inodeNum += inodesPerGroup;
		blockNum += blocksPerGroup;
		(groupDesc + i)->availInodeNum = inodesPerGroup;
		(groupDesc + i)->availBlockNum = blocksPerGroup;
		(groupDesc + i)->inodeBitmap = counter + inodeBitmapOffset; 
		(groupDesc + i)->blockBitmap = counter + blockBitmapOffset;
		(groupDesc + i)->inodeTable = counter + inodeTableOffset;
		counter += groupSize;
	}
	superBlock->sectorNum = sectorNum;
	superBlock->inodeNum = inodeNum;
	superBlock->blockNum = blockNum;
	superBlock->availInodeNum = inodeNum;
	superBlock->availBlockNum = blockNum;
	superBlock->blockSize = SECTOR_SIZE * sectorsPerBlock;
	superBlock->inodesPerGroup = SECTOR_SIZE * sectorsPerBlock * 8;
	superBlock->blocksPerGroup = SECTOR_SIZE * sectorsPerBlock * 8;
	for (i = 0; i < groupNum; i ++) {
		//fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		//fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
		diskWrite((void*)superBlock, sizeof(SuperBlock), 1, i * groupSize * SECTOR_SIZE);
		diskWrite((void*)groupDesc, sizeof(GroupDesc), groupNum, i * groupSize * SECTOR_SIZE + sizeof(SuperBlock) * 1);
	}
	return 0;
}

//int readGroupHeader (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc) {
int readGroupHeader (SuperBlock *superBlock, GroupDesc *groupDesc) {
	int groupNum = 0;
	//fseek(file, 0, SEEK_SET);
	//fread((void*)superBlock, sizeof(SuperBlock), 1, file);
	diskRead((void*)superBlock, sizeof(SuperBlock), 1, 0);
	groupNum = calGroupNum(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE);
	if (groupNum == 0)
		return -1;
	//fread((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	diskRead((void*)groupDesc, sizeof(GroupDesc), groupNum, sizeof(SuperBlock));
	return 0;
}

/* do not check if blockIndex is less than inode->blockCount */
//int readBlock (FILE *file, SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer) {
int readBlock (SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer) {
	int divider0 = superBlock->blockSize / 4;
	int divider1 = divider0 * divider0;
	int divider2 = divider1 * divider0;
	int bound0 = POINTER_NUM;
	int bound1 = bound0 + divider0;
	int bound2 = bound1 + divider1;
	int bound3 = bound2 + divider2;

	uint32_t singlyPointerBuffer[divider0];
	uint32_t doublyPointerBuffer[divider0];
	uint32_t triplyPointerBuffer[divider0];
	int quotient = 0;
	int remainder = 0;
	if (blockIndex < bound0) {
		//fseek(file, inode->pointer[blockIndex] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)buffer, sizeof(uint8_t), superBlock->blockSize, inode->pointer[blockIndex] * SECTOR_SIZE);
		return 0;
	}
	else if (blockIndex < bound1) {
		//fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->singlyPointer * SECTOR_SIZE);
		//fseek(file, singlyPointerBuffer[blockIndex - bound0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)buffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBuffer[blockIndex - bound0] * SECTOR_SIZE);
		return 0;
	}
	else if (blockIndex < bound2) {
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
		quotient = (blockIndex - bound1) / divider0;
		remainder = (blockIndex - bound1) % divider0;
		//fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[quotient] * SECTOR_SIZE);
		//fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)buffer, sizeof(uint8_t) , superBlock->blockSize, file);
		diskRead((void*)buffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBuffer[remainder] * SECTOR_SIZE);
		return 0;
	}
	else if (blockIndex < bound3) {
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		quotient = (blockIndex - bound2) / divider1;
		remainder = (blockIndex - bound2) % divider1;
		//fseek(file, triplyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[quotient] * SECTOR_SIZE);
		quotient = remainder / divider0;
		remainder = remainder % divider0;
		//fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[quotient] * SECTOR_SIZE);
		//fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)buffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBuffer[remainder] * SECTOR_SIZE);
		return 0;
	}
	else
		return -1;
}

/* do not check if blockIndex is less than inode->blockCount */
//int writeBlock (FILE *file, SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer) {
int writeBlock (SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer) {
	int divider0 = superBlock->blockSize / 4;
	int divider1 = divider0 * divider0;
	int divider2 = divider1 * divider0;
	int bound0 = POINTER_NUM;
	int bound1 = bound0 + divider0;
	int bound2 = bound1 + divider1;
	int bound3 = bound2 + divider2;

	uint32_t singlyPointerBuffer[divider0];
	uint32_t doublyPointerBuffer[divider0];
	uint32_t triplyPointerBuffer[divider0];
	int quotient = 0;
	int remainder = 0;
	if (blockIndex < bound0) {
		//fseek(file, inode->pointer[blockIndex] * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, inode->pointer[blockIndex] * SECTOR_SIZE);
		return 0;
	}
	else if (blockIndex < bound1) {
		//fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->singlyPointer * SECTOR_SIZE);
		//fseek(file, singlyPointerBuffer[blockIndex - bound0] * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBuffer[blockIndex - bound0] * SECTOR_SIZE);
		return 0;
	}
	else if (blockIndex < bound2) {
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
		quotient = (blockIndex - bound1) / divider0;
		remainder = (blockIndex - bound1) % divider0;
		//fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[quotient] * SECTOR_SIZE);
		//fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)buffer, sizeof(uint8_t) , superBlock->blockSize, file);
		diskWrite((void*)buffer, sizeof(uint8_t) , superBlock->blockSize, singlyPointerBuffer[remainder] * SECTOR_SIZE);
		return 0;
	}
	else if (blockIndex < bound3) {
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		quotient = (blockIndex - bound2) / divider1;
		remainder = (blockIndex - bound2) % divider1;
		//fseek(file, triplyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[quotient] * SECTOR_SIZE);
		quotient = remainder / divider0;
		remainder = remainder % divider0;
		//fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[quotient] * SECTOR_SIZE);
		//fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBuffer[remainder] * SECTOR_SIZE);
		return 0;
	}
	else
		return -1;
}

/*
* return the number of pointerBlock should be allocated in addition if the blockCount-th block is added
*/
int calNeededPointerBlocks (SuperBlock *superBlock, int blockCount) {
	int divider0 = superBlock->blockSize / 4;
	int divider1 = divider0 * divider0;
	int divider2 = divider1 * divider0;
	int bound0 = POINTER_NUM;
	int bound1 = bound0 + divider0;
	int bound2 = bound1 + divider1;
	int bound3 = bound2 + divider2;

	if (blockCount == bound0)
		return 1;
	else if (blockCount == bound1)
		return 2;
	else if (blockCount < bound2 && (blockCount - bound1) % divider0 == 0)
		return 1;
	else if (blockCount == bound2)
		return 3;
	else if (blockCount < bound3 && (blockCount - bound2) % divider1 == 0)
		return 2;
	else if (blockCount < bound3 && (blockCount - bound2) % divider0 == 0)
		return 1;
	else if (blockCount >= bound3)
		return -1;
	else
		return 0;
}

/*
* get the blockOffset of 1 available block, write back the correlated blockBitmap, superBlock, groupDesc
* getAvailBlock is safe, i.e., superBlock, groupDesc, blockOffset won't be modified if getAvailBlock fails
*/
//int getAvailBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int *blockOffset) {
int getAvailBlock (SuperBlock *superBlock, GroupDesc *groupDesc, int *blockOffset) {
	int i = 0;
	int j = 0;
	int k = 0;
	int groupNum = calGroupNum(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE);
	int groupSize = calGroupSize(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE, groupNum, 0);
	int blockBitmapOffset = 0;
	BlockBitmap blockBitmap;
	
	if (superBlock->availBlockNum == 0)
		return -1;
	superBlock->availBlockNum --; // superBlock is updated

	for (i = 0; i < groupNum; i ++) {
		if ((groupDesc + i)->availBlockNum >= 1) {
			blockBitmapOffset = (groupDesc + i)->blockBitmap;
			*blockOffset = (groupDesc + i)->inodeTable;
			break;
		}
	}
	//fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fread((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // read blockBitmap
	diskRead((void*)&blockBitmap, sizeof(BlockBitmap), 1, blockBitmapOffset * SECTOR_SIZE);
	for (j = 0; j < ((groupDesc + i)->availBlockNum + 7)/ 8; j ++)
		if (blockBitmap.byte[j] !=0xff)
			break;
	for (k = 0; k < 8; k ++)
		if ((blockBitmap.byte[j] >> (7-k)) % 2 == 0)
			break;
	blockBitmap.byte[j] = blockBitmap.byte[j] | (1 << (7-k));
	*blockOffset += sizeof(Inode) * 8 * superBlock->blockSize / SECTOR_SIZE + (j * 8 + k) * superBlock->blockSize / SECTOR_SIZE; // sector as unit
	(groupDesc + i)->availBlockNum --; // groupDesc is updated

	/* write superBlock back
	 *    write groupDesc back */
	for (i = 0; i < groupNum; i ++) {
		//fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		//fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
		diskWrite((void*)superBlock, sizeof(SuperBlock), 1, i * groupSize * SECTOR_SIZE);
		diskWrite((void*)groupDesc, sizeof(GroupDesc), groupNum, i * groupSize * SECTOR_SIZE + sizeof(SuperBlock) * 1);
	}
	/* write blockBitmap back */
	//fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fwrite((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // write whole blockBitmap
	diskWrite((void*)&blockBitmap, sizeof(BlockBitmap), 1, blockBitmapOffset * SECTOR_SIZE);
	
	return 0;
}

//int setAllocBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int blockOffset) {
int setAllocBlock (SuperBlock *superBlock, GroupDesc *groupDesc, int blockOffset) {
	int i = 0;
	int j = 0;
	int k = 0;
	int groupNum = calGroupNum(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE);
	int groupSize = calGroupSize(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE, groupNum, 0);
	int blockBitmapOffset = 0;
	BlockBitmap blockBitmap;

	i = blockOffset / groupSize;
	j = (blockOffset - (groupDesc + i)->inodeTable - sizeof(Inode) * 8 * superBlock->blockSize / SECTOR_SIZE) / (superBlock->blockSize / SECTOR_SIZE) / 8;
	k = (blockOffset - (groupDesc + i)->inodeTable - sizeof(Inode) * 8 * superBlock->blockSize / SECTOR_SIZE) / (superBlock->blockSize / SECTOR_SIZE) % 8;

	blockBitmapOffset = (groupDesc + i)->blockBitmap;
	//fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fread((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // read blockBitmap
	diskRead((void*)&blockBitmap, sizeof(BlockBitmap), 1, blockBitmapOffset * SECTOR_SIZE);
	if ((blockBitmap.byte[j] >> (7-k)) % 2 == 0)
		return -1;

	superBlock->availBlockNum ++; // superBlock is updated
	(groupDesc + i)->availBlockNum ++; // groupDesc is updated
	blockBitmap.byte[j] = blockBitmap.byte[j] ^ (1 << (7-k));
	
	/* write superBlock back
	 *    write groupDesc back */
	for (i = 0; i < groupNum; i ++) {
		//fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		//fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
		diskWrite((void*)superBlock, sizeof(SuperBlock), 1, i * groupSize * SECTOR_SIZE);
		diskWrite((void*)groupDesc, sizeof(GroupDesc), groupNum, i * groupSize * SECTOR_SIZE + sizeof(SuperBlock) * 1);
	}
	/* write blockBitmap back */
	//fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fwrite((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // write whole blockBitmap
	diskWrite((void*)&blockBitmap, sizeof(BlockBitmap), 1, blockBitmapOffset * SECTOR_SIZE);
	
	return 0;
}

/* allocLastBlock is not safe, as getAvailBlock is verified */
//int allocLastBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset, int blockOffset) {
int allocLastBlock (SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset, int blockOffset) {
	int divider0 = superBlock->blockSize / 4;
	int divider1 = divider0 * divider0;
	int divider2 = divider1 * divider0;
	int bound0 = POINTER_NUM;
	int bound1 = bound0 + divider0;
	int bound2 = bound1 + divider1;
	int bound3 = bound2 + divider2;
	
	uint32_t singlyPointerBuffer[divider0];
	uint32_t doublyPointerBuffer[divider0];
	uint32_t triplyPointerBuffer[divider0];
	int singlyPointerBufferOffset = 0;
	int doublyPointerBufferOffset = 0;
	int triplyPointerBufferOffset = 0;
	
	/* need to write back pointerblock */
	if (inode->blockCount < bound0) {
		inode->pointer[inode->blockCount] = blockOffset;
	}
	else if (inode->blockCount == bound0) {
		//getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &singlyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		inode->singlyPointer = singlyPointerBufferOffset;
		// write back
		//fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBufferOffset * SECTOR_SIZE);
	}
	else if (inode->blockCount < bound1) {
		//fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->singlyPointer * SECTOR_SIZE);
		singlyPointerBuffer[inode->blockCount - bound0] = blockOffset;
		// write back
		//fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->singlyPointer * SECTOR_SIZE);
	}
	else if (inode->blockCount == bound1) {
		//getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &singlyPointerBufferOffset);
		//getAvailBlock(file, superBlock, groupDesc, &doublyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &doublyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		doublyPointerBuffer[0] = singlyPointerBufferOffset;
		inode->doublyPointer = doublyPointerBufferOffset;
		// write back
		//fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBufferOffset * SECTOR_SIZE);
		//fseek(file, doublyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBufferOffset * SECTOR_SIZE);

	}
	else if (inode->blockCount < bound2 && (inode->blockCount - bound1) % divider0 == 0) {
		//getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &singlyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
		doublyPointerBuffer[(inode->blockCount - bound1) / divider0] = singlyPointerBufferOffset;
		// write back
		//fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBufferOffset * SECTOR_SIZE);
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
	}
	else if (inode->blockCount < bound2) {
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE);
		singlyPointerBuffer[(inode->blockCount - bound1) % divider0] = blockOffset;
		// write back
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE);
	}
	else if (inode->blockCount == bound2) {
		//getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &singlyPointerBufferOffset);
		//getAvailBlock(file, superBlock, groupDesc, &doublyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &doublyPointerBufferOffset);
		//getAvailBlock(file, superBlock, groupDesc, &triplyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &triplyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		doublyPointerBuffer[0] = singlyPointerBufferOffset;
		triplyPointerBuffer[0] = doublyPointerBufferOffset;
		inode->triplyPointer = triplyPointerBufferOffset;
		// write back
		//fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBufferOffset * SECTOR_SIZE);
		//fseek(file, doublyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBufferOffset * SECTOR_SIZE);
		//fseek(file, triplyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBufferOffset * SECTOR_SIZE);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider1 == 0) {
		//getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &singlyPointerBufferOffset);
		//getAvailBlock(file, superBlock, groupDesc, &doublyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &doublyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		doublyPointerBuffer[0] = singlyPointerBufferOffset;
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		triplyPointerBuffer[(inode->blockCount - bound2) / divider1] = doublyPointerBufferOffset;
		// write back
		//fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBufferOffset * SECTOR_SIZE);
		//fseek(file, doublyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBufferOffset * SECTOR_SIZE);
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider0 == 0) {
		//getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(superBlock, groupDesc, &singlyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		//fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE);
		doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] = singlyPointerBufferOffset;
		// write back
		//fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, singlyPointerBufferOffset * SECTOR_SIZE);
		//fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE);
	}
	else if (inode->blockCount < bound3) {
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		//fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE);
		singlyPointerBuffer[(inode->blockCount - bound2) % divider1 % divider0] = blockOffset;
		// write back
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskWrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE);
	}
	else
		return -1;
	// write back inode
	inode->blockCount ++;
	//fseek(file, inodeOffset, SEEK_SET);
	//fwrite((void*)inode, sizeof(Inode), 1, file);
	diskWrite((void*)inode, sizeof(Inode), 1, inodeOffset);
	return 0;
}

/*
* to allocate 1 more block, 4 more blocks may be allocated in total in the worst case (i.e., 3 for pointer, and 1 for regular data
* allocBlock is safe, i.e., file, superBlock, groupDesc, inode, blockOffset won't be modified if allocBlock fails
*/
//int allocBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
int allocBlock (SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
	int ret = 0;
	int blockOffset = 0;
	
	/* verify whether allocBlock would fail or not */
	ret = calNeededPointerBlocks(superBlock, inode->blockCount);
	if (superBlock->availBlockNum < ret + 1) // ret + 1 number of blocks should be allocated in total
		return -1;
	
	/* get the block to allocate */
	//getAvailBlock(file, superBlock, groupDesc, &blockOffset);
	getAvailBlock(superBlock, groupDesc, &blockOffset);

	/* allocte pointer block of inode */
	//allocLastBlock(file, superBlock, groupDesc, inode, inodeOffset, blockOffset);
	allocLastBlock(superBlock, groupDesc, inode, inodeOffset, blockOffset);
	
	return 0;
}

/*
* free the last block of inode
*/
//int freeLastBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
int freeLastBlock (SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
	int divider0 = superBlock->blockSize / 4;
	int divider1 = divider0 * divider0;
	int divider2 = divider1 * divider0;
	int bound0 = POINTER_NUM;
	int bound1 = bound0 + divider0;
	int bound2 = bound1 + divider1;
	int bound3 = bound2 + divider2;
	
	uint32_t singlyPointerBuffer[divider0];
	uint32_t doublyPointerBuffer[divider0];
	uint32_t triplyPointerBuffer[divider0];

	// setting inode
	inode->blockCount --;

	if (inode->blockCount < bound0) {
		//setAllocBlock(file, superBlock, groupDesc, inode->pointer[inode->blockCount]);
		setAllocBlock(superBlock, groupDesc, inode->pointer[inode->blockCount]);
	}
	else if (inode->blockCount == bound0) {
		//fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->singlyPointer * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, inode->singlyPointer);
		setAllocBlock(superBlock, groupDesc, inode->singlyPointer);
	}
	else if (inode->blockCount < bound1) {
		//fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->singlyPointer * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[inode->blockCount - bound0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[inode->blockCount - bound0]);
	}
	else if (inode->blockCount == bound1) {
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[0] * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, doublyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, inode->doublyPointer);
		setAllocBlock(superBlock, groupDesc, inode->doublyPointer);
	}
	else if (inode->blockCount < bound2 && (inode->blockCount - bound1) % divider0 == 0) {
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[(inode->blockCount - bound1) / divider0]);
		setAllocBlock(superBlock, groupDesc, doublyPointerBuffer[(inode->blockCount - bound1) / divider0]);
	}
	else if (inode->blockCount < bound2) {
		//fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->doublyPointer * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[(inode->blockCount - bound1) % divider0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[(inode->blockCount - bound1) % divider0]);
	}
	else if (inode->blockCount == bound2) {
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		//fseek(file, triplyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[0] * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[0] * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, doublyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, triplyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, triplyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, inode->triplyPointer);
		setAllocBlock(superBlock, groupDesc, inode->triplyPointer);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider1 == 0) {
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		//fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[0] * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, doublyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, triplyPointerBuffer[(inode->blockCount - bound2) / divider1]);
		setAllocBlock(superBlock, groupDesc, triplyPointerBuffer[(inode->blockCount - bound2) / divider1]);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider0 == 0) {
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		//fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[0]);
		//setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0]);
		setAllocBlock(superBlock, groupDesc, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0]);
	}
	else if (inode->blockCount < bound3) {
		//fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		//fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, inode->triplyPointer * SECTOR_SIZE);
		//fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE);
		//fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		//fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		diskRead((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE);
		//setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[(inode->blockCount - bound2) % divider1 % divider0]);
		setAllocBlock(superBlock, groupDesc, singlyPointerBuffer[(inode->blockCount - bound2) % divider1 % divider0]);
	}
	else
		return -1;
	// write back inode
	//fseek(file, inodeOffset, SEEK_SET);
	//fwrite((void*)inode, sizeof(Inode), 1, file);
	diskWrite((void*)inode, sizeof(Inode), 1, inodeOffset);
	return 0;
}

/*
* to free all block of inode
*/
//int freeBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
int freeBlock (SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
	int ret = 0;
	while (inode->blockCount != 0) {
		//ret = freeLastBlock(file, superBlock, groupDesc, inode, inodeOffset);
		ret = freeLastBlock(superBlock, groupDesc, inode, inodeOffset);
		if (ret == -1)
			return -1;
		//printf("FreeBlock.\n%d inodes and %d data blocks available.\n", superBlock->availInodeNum, superBlock->availBlockNum);
	}
	return 0;
}

//int getDirEntry (FILE *file, SuperBlock *superBlock, Inode *inode, int dirIndex, DirEntry *destDirEntry) {
int getDirEntry (SuperBlock *superBlock, Inode *inode, int dirIndex, DirEntry *destDirEntry) {
	int i = 0;
	int j = 0;
	int ret = 0;
	int dirCount = 0;
	DirEntry *dirEntry = NULL;
	uint8_t buffer[superBlock->blockSize];

	for (i = 0; i < inode->blockCount; i ++) {
		//ret = readBlock(file, superBlock, inode, i, buffer);
		ret = readBlock(superBlock, inode, i, buffer);
		if (ret == -1)
			return -1;
		dirEntry = (DirEntry*)buffer;
		for (j = 0; j < superBlock->blockSize / sizeof(DirEntry); j ++) {
			if (dirEntry[j].inode != 0) {
				if (dirCount == dirIndex)
					break;
				else
					dirCount ++;
			}
		}
		if (j < superBlock->blockSize / sizeof(DirEntry))
			break;
	}
	if (i == inode->blockCount)
		return -1;
	else {
		destDirEntry->inode = dirEntry[j].inode;
		stringCpy(dirEntry[j].name, destDirEntry->name, NAME_LENGTH);
		return 0;
	}
}

/*
* Supported destFilePath pattern
* destFilePath = "" empty string, echo error
* destFilePath = "/xxx//xxx", echo error
* destFilePath = "xxx/...", echo error
* destFilePath = "/" for root directory
* destFilePath = "/xxx/.../xxxx" for both directory and regular file
* destFilePath = "/xxx/.../xxxx/" for directory file only
*/
//int readInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc,
int readInode (SuperBlock *superBlock, GroupDesc *groupDesc,
		Inode *destInode, int *inodeOffset, const char *destFilePath) {
	int i = 0;
	int j = 0;
	int ret = 0;
	int cond = 0; // label to mark if destFilePath is ended with '/'
	*inodeOffset = 0;
	uint8_t buffer[superBlock->blockSize];
	DirEntry *dirEntry = NULL;
	int count = 0; // index of destFilePath
	int size = 0; // size of filename
	int blockCount = 0;

	if (destFilePath == NULL || destFilePath[count] == 0)
		return -1;

	ret = stringChr(destFilePath, '/', &size);
	if (ret == -1 || size != 0) // not started with '/'
		return -1;

	count += size + 1;
	*inodeOffset = groupDesc->inodeTable * SECTOR_SIZE; // inodeOffset of '/'
	//fseek(file, *inodeOffset, SEEK_SET);
	//fread((void*)destInode, sizeof(Inode), 1, file);
	diskRead((void*)destInode, sizeof(Inode), 1, *inodeOffset);

	while(destFilePath[count] != 0) { // not empty
		ret = stringChr(destFilePath + count, '/', &size);
		if (ret == 0 && size == 0) // pattern '//' occured
			return -1;
		if (ret == -1) // no more '/'
			cond = 1;
		else if (destInode->type == REGULAR_TYPE) // with more '/' but regular file
			return -1;
		blockCount = destInode->blockCount;
		for (i = 0; i < blockCount; i ++) {
			//ret = readBlock(file, superBlock, destInode, i, buffer);
			ret = readBlock(superBlock, destInode, i, buffer);
			if (ret == -1)
				return -1;
			dirEntry = (DirEntry*)buffer;
			for (j = 0; j < superBlock->blockSize / sizeof(DirEntry); j ++) {
				if (dirEntry[j].inode == 0) // empty dirEntry
					continue;
				else if (stringCmp(dirEntry[j].name, destFilePath + count, size) == 0) {
					*inodeOffset = dirEntry[j].inode;
					//fseek(file, *inodeOffset, SEEK_SET);
					//fread((void*)destInode, sizeof(Inode), 1, file);
					diskRead((void*)destInode, sizeof(Inode), 1, *inodeOffset);
					break;
				}
			}
			if (j < superBlock->blockSize / sizeof(DirEntry))
				break;
		}
		if (i < blockCount) {
			if (cond == 0)
				count += size + 1;
			else
				return 0;
		}
		else
			return -1;
	}
	return 0;
}

/*
* get the inodeOffset of 1 available inode, write back the correlated inodeBitmap, superBlock, groupDesc
* getAvailInode is safe, i.e., superBlock, groupDesc, inodeOffset won't be modified if getAvailInode fails
*/
//int getAvailInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int *inodeOffset) {
int getAvailInode (SuperBlock *superBlock, GroupDesc *groupDesc, int *inodeOffset) {
	int i = 0;
	int j = 0;
	int k = 0;
	int groupNum = calGroupNum(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE);
	int groupSize = calGroupSize(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE, groupNum, 0);
	int inodeBitmapOffset = 0;
	int inodeTableOffset = 0;
	InodeBitmap inodeBitmap;
	
	/* check inode & block available or not */
	if (superBlock->availInodeNum == 0)
		return -1;
	superBlock->availInodeNum --; // superBlock is updated

	/* for a directory file, allocate a new inode */
	for (i = 0; i < groupNum; i ++) {
		if ((groupDesc + i)->availInodeNum >= 1) {
			inodeBitmapOffset = (groupDesc + i)->inodeBitmap;
			inodeTableOffset = (groupDesc + i)->inodeTable;
			break;
		}
	}
	//fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fread((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file); // read inodeBitmap
	diskRead((void*)&inodeBitmap, sizeof(InodeBitmap), 1, inodeBitmapOffset * SECTOR_SIZE);
	for (j = 0; j < ((groupDesc + i)->availInodeNum + 7)/ 8; j ++)
		if (inodeBitmap.byte[j] != 0xff)
			break;
	for (k = 0; k < 8; k ++)
		if ((inodeBitmap.byte[j] >> (7-k)) % 2 == 0)
			break;
	inodeBitmap.byte[j] = inodeBitmap.byte[j] | (1 << (7-k));
	*inodeOffset = inodeTableOffset * SECTOR_SIZE + (j * 8 + k) * sizeof(Inode); // byte as unit
	(groupDesc + i)->availInodeNum --; // groupDesc is updated

	/* write superBlock back
	 * write groupDesc back */
	for (i = 0; i < groupNum; i ++) {
		//fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		//fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
		diskWrite((void*)superBlock, sizeof(SuperBlock), 1, i * groupSize * SECTOR_SIZE);
		diskWrite((void*)groupDesc, sizeof(GroupDesc), groupNum, i * groupSize * SECTOR_SIZE + sizeof(SuperBlock) * 1);
	}

	/* write inodeBitmap back */
	//fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fwrite((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file); // write whole inodeBitmap
	diskWrite((void*)&inodeBitmap, sizeof(InodeBitmap), 1, inodeBitmapOffset * SECTOR_SIZE);

	return 0;
}

//int setAllocInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int inodeOffset) {
int setAllocInode (SuperBlock *superBlock, GroupDesc *groupDesc, int inodeOffset) {
	int i = 0;
	int j = 0;
	int k = 0;
	int groupNum = calGroupNum(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE);
	int groupSize = calGroupSize(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE, groupNum, 0);
	int inodeBitmapOffset = 0;
	InodeBitmap inodeBitmap;

	i = inodeOffset / SECTOR_SIZE / groupSize;
	j = (inodeOffset - (groupDesc + i)->inodeTable * SECTOR_SIZE) / sizeof(Inode) / 8;
	k = (inodeOffset - (groupDesc + i)->inodeTable * SECTOR_SIZE) / sizeof(Inode) % 8;

	inodeBitmapOffset = (groupDesc + i)->inodeBitmap;
	//fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fread((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file);
	diskRead((void*)&inodeBitmap, sizeof(InodeBitmap), 1, inodeBitmapOffset * SECTOR_SIZE);
	if ((inodeBitmap.byte[j] >> (7-k)) % 2 == 0)
		return -1;

	superBlock->availInodeNum ++;
	(groupDesc + i)->availInodeNum ++;
	inodeBitmap.byte[j] = inodeBitmap.byte[j] ^ (1 << (7-k));

	/* write superBlock back
	 *    write groupDesc back */
	for (i = 0; i < groupNum; i ++) {
		//fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		//fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		//fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
		diskWrite((void*)superBlock, sizeof(SuperBlock), 1, i * groupSize * SECTOR_SIZE);
		diskWrite((void*)groupDesc, sizeof(GroupDesc), groupNum, i * groupSize * SECTOR_SIZE + sizeof(SuperBlock) * 1);
	}
	/* write inodeBitmap back */
	//fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	//fwrite((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file); // write whole blockBitmap
	diskWrite((void*)&inodeBitmap, sizeof(InodeBitmap), 1, inodeBitmapOffset * SECTOR_SIZE);
	
	return 0;
}

/*
* Supported destFilename pattern
* destFilename = "" empty string, echo error
*/
//int allocInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc,
int allocInode (SuperBlock *superBlock, GroupDesc *groupDesc,
		Inode *fatherInode, int fatherInodeOffset,
		Inode *destInode, int *destInodeOffset, const char *destFilename, int destFiletype) {
	int i = 0;
	int j = 0;
	int ret = 0;
	DirEntry *dirEntry = NULL;
	uint8_t buffer[superBlock->blockSize];
	int length = stringLen(destFilename);

	if (destFilename == NULL || destFilename[0] == 0)
		return -1;

	/* verify whether available inode exist*/
	if (superBlock->availInodeNum == 0)
		return -1;
	/* setting dirEntry */
	for (i = 0; i < fatherInode->blockCount; i ++) {
		//ret = readBlock(file, superBlock, fatherInode, i, buffer);
		ret = readBlock(superBlock, fatherInode, i, buffer);
		if (ret == -1)
			return -1;
		dirEntry = (DirEntry*)buffer;
		for (j = 0; j < superBlock->blockSize / sizeof(DirEntry); j ++) {
			if (dirEntry[j].inode == 0) // a valid empty dirEntry
				break;
			else if (stringCmp(dirEntry[j].name, destFilename, length) == 0)
				return -1; // file with filename = destFilename exist
		}
		if (j < superBlock->blockSize / sizeof(DirEntry))
			break;
	}
	if (i == fatherInode->blockCount) { // allocate a new data block for fatherInode
		//ret = allocBlock(file, superBlock, groupDesc, fatherInode, fatherInodeOffset);
		ret = allocBlock(superBlock, groupDesc, fatherInode, fatherInodeOffset);
		if (ret == -1)
			return -1;
		fatherInode->size = fatherInode->blockCount * superBlock->blockSize; // need to write back fatherInode
		setBuffer(buffer, superBlock->blockSize, 0); // reset buffer
		dirEntry = (DirEntry*)buffer;
		j = 0;
	}
	/* dirEntry[j] is the valid empty dirEntry, it is in the i-th block of fatherInode */
	/* get available inode */
	//ret = getAvailInode(file, superBlock, groupDesc, destInodeOffset);
	ret = getAvailInode(superBlock, groupDesc, destInodeOffset);
	if (ret == -1) // as we have check it in advance, it won't happen
		return -1;
	/* setting dirEntry */
	stringCpy(destFilename, dirEntry[j].name, NAME_LENGTH); // setting name of dirEntry
	dirEntry[j].inode = *destInodeOffset; // setting inode of dirEntry
	/* write back dirEntry */
	//ret = writeBlock(file, superBlock, fatherInode, i, buffer); // write back i-th block of fatherInode
	ret = writeBlock(superBlock, fatherInode, i, buffer); // write back i-th block of fatherInode
	if (ret == -1) // normally, it won't happen
		return -1;
	/* write back fatherInode */
	//fseek(file, fatherInodeOffset, SEEK_SET);
	//fwrite((void*)fatherInode, sizeof(Inode), 1, file);
	diskWrite((void*)fatherInode, sizeof(Inode), 1, fatherInodeOffset);
	/* setting destInode */
	destInode->type = destFiletype;
	destInode->linkCount = 1;
	destInode->blockCount = 0;
	destInode->size = 0;
	/* write back destInode */
	//fseek(file, *destInodeOffset, SEEK_SET);
	//fwrite((void*)destInode, sizeof(Inode), 1, file);
	diskWrite((void*)destInode, sizeof(Inode), 1, *destInodeOffset);

	return 0;
}

//int freeInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc,
int freeInode (SuperBlock *superBlock, GroupDesc *groupDesc,
		Inode *fatherInode, int fatherInodeOffset,
		Inode *destInode, int *destInodeOffset, const char *destFilename, int destFiletype) {
	int i = 0;
	int j = 0;
	int ret = 0;
	DirEntry *dirEntry = NULL;
	uint8_t buffer[superBlock->blockSize];
	int length = stringLen(destFilename);
	DirEntry tmpDirEntry;

	if (destFilename == NULL || destFilename[0] == 0)
		return -1;
	
	for (i = 0; i < fatherInode->blockCount; i ++) {
		//ret = readBlock(file, superBlock, fatherInode, i, buffer);
		ret = readBlock(superBlock, fatherInode, i, buffer);
		if (ret == -1)
			return -1;
		dirEntry = (DirEntry*)buffer;
		for (j = 0; j < superBlock->blockSize / sizeof(DirEntry); j ++) {
			if (dirEntry[j].inode == 0)
				continue;
			else if (stringCmp(dirEntry[j].name, destFilename, length) == 0)
				break;
		}
		if (j < superBlock->blockSize / sizeof(DirEntry))
			break;
	}
	if (i == fatherInode->blockCount)
		return -1;
	/* free destInode */
	*destInodeOffset = dirEntry[j].inode;
	//fseek(file, *destInodeOffset, SEEK_SET);
	//fread((void*)destInode, sizeof(Inode), 1, file);
	diskRead((void*)destInode, sizeof(Inode), 1, *destInodeOffset);
	if (destInode->type != destFiletype) // verify the filetype
		return -1;
	if (destFiletype == DIRECTORY_TYPE) { // check if the directory is empty
		//if (getDirEntry(file, superBlock, destInode, 0, &tmpDirEntry) != -1) // not empty
		if (getDirEntry(superBlock, destInode, 0, &tmpDirEntry) != -1) // not empty
			return -1;
	}
	destInode->linkCount --;
	if (destInode->linkCount == 0) {
		/* setting inodeBitmap, groupDesc, superBlock
		 * write back inodeBitmap, groupDesc, superBlock */
		//freeBlock(file, superBlock, groupDesc, destInode, *destInodeOffset);
		freeBlock(superBlock, groupDesc, destInode, *destInodeOffset);
		//setAllocInode(file, superBlock, groupDesc, *destInodeOffset);
		setAllocInode(superBlock, groupDesc, *destInodeOffset);
	}
	else {
		/* write back destInode */
		//fseek(file, *destInodeOffset, SEEK_SET);
		//fwrite((void*)destInode, sizeof(Inode), 1, file);
		diskWrite((void*)destInode, sizeof(Inode), 1, *destInodeOffset);
	}
	/* setting dirEntry[j] in i-th block of fatherInode*/
	dirEntry[j].inode = 0;
	/* write back i-th block of fatherInode */
	//ret = writeBlock(file, superBlock, fatherInode, i, buffer);
	ret = writeBlock(superBlock, fatherInode, i, buffer);

	return 0;
}
