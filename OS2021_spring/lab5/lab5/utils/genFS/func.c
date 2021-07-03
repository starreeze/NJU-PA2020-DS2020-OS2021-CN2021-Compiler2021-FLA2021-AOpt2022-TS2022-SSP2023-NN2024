#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "data.h"
#include "func.h"

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

int initGroupHeader (FILE *file, int sectorNum, int sectorsPerBlock, SuperBlock *superBlock, GroupDesc *groupDesc) {
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
		fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	}
	return 0;
}

int readGroupHeader (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc) {
	int groupNum = 0;
	fseek(file, 0, SEEK_SET);
	fread((void*)superBlock, sizeof(SuperBlock), 1, file);
	groupNum = calGroupNum(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE);
	if (groupNum == 0)
		return -1;
	fread((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	return 0;
}

/* do not check if blockIndex is less than inode->blockCount */
int readBlock (FILE *file, SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer) {
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
		fseek(file, inode->pointer[blockIndex] * SECTOR_SIZE, SEEK_SET);
		fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		return 0;
	}
	else if (blockIndex < bound1) {
		fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, singlyPointerBuffer[blockIndex - bound0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		return 0;
	}
	else if (blockIndex < bound2) {
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		quotient = (blockIndex - bound1) / divider0;
		remainder = (blockIndex - bound1) % divider0;
		fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		fread((void*)buffer, sizeof(uint8_t) , superBlock->blockSize, file);
		return 0;
	}
	else if (blockIndex < bound3) {
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		quotient = (blockIndex - bound2) / divider1;
		remainder = (blockIndex - bound2) % divider1;
		fseek(file, triplyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		quotient = remainder / divider0;
		remainder = remainder % divider0;
		fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		return 0;
	}
	else
		return -1;
}

/* do not check if blockIndex is less than inode->blockCount */
int writeBlock (FILE *file, SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer) {
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
		fseek(file, inode->pointer[blockIndex] * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		return 0;
	}
	else if (blockIndex < bound1) {
		fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, singlyPointerBuffer[blockIndex - bound0] * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
		return 0;
	}
	else if (blockIndex < bound2) {
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		quotient = (blockIndex - bound1) / divider0;
		remainder = (blockIndex - bound1) % divider0;
		fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)buffer, sizeof(uint8_t) , superBlock->blockSize, file);
		return 0;
	}
	else if (blockIndex < bound3) {
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		quotient = (blockIndex - bound2) / divider1;
		remainder = (blockIndex - bound2) % divider1;
		fseek(file, triplyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		quotient = remainder / divider0;
		remainder = remainder % divider0;
		fseek(file, doublyPointerBuffer[quotient] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, singlyPointerBuffer[remainder] * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)buffer, sizeof(uint8_t), superBlock->blockSize, file);
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
int getAvailBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int *blockOffset) {
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
	fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fread((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // read blockBitmap
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
		fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	}
	/* write blockBitmap back */
	fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fwrite((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // write whole blockBitmap
	
	return 0;
}

int setAllocBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int blockOffset) {
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
	fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fread((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // read blockBitmap
	if ((blockBitmap.byte[j] >> (7-k)) % 2 == 0)
		return -1;

	superBlock->availBlockNum ++; // superBlock is updated
	(groupDesc + i)->availBlockNum ++; // groupDesc is updated
	blockBitmap.byte[j] = blockBitmap.byte[j] ^ (1 << (7-k));
	
	/* write superBlock back
	 *    write groupDesc back */
	for (i = 0; i < groupNum; i ++) {
		fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	}
	/* write blockBitmap back */
	fseek(file, blockBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fwrite((void*)&blockBitmap, sizeof(BlockBitmap), 1, file); // write whole blockBitmap
	
	return 0;
}

/* allocLastBlock is not safe, as getAvailBlock is verified */
int allocLastBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset, int blockOffset) {
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
		getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		inode->singlyPointer = singlyPointerBufferOffset;
		// write back
		fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount < bound1) {
		fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		singlyPointerBuffer[inode->blockCount - bound0] = blockOffset;
		// write back
		fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount == bound1) {
		getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(file, superBlock, groupDesc, &doublyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		doublyPointerBuffer[0] = singlyPointerBufferOffset;
		inode->doublyPointer = doublyPointerBufferOffset;
		// write back
		fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount < bound2 && (inode->blockCount - bound1) % divider0 == 0) {
		getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		doublyPointerBuffer[(inode->blockCount - bound1) / divider0] = singlyPointerBufferOffset;
		// write back
		fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount < bound2) {
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		singlyPointerBuffer[(inode->blockCount - bound1) % divider0] = blockOffset;
		// write back
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount == bound2) {
		getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(file, superBlock, groupDesc, &doublyPointerBufferOffset);
		getAvailBlock(file, superBlock, groupDesc, &triplyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		doublyPointerBuffer[0] = singlyPointerBufferOffset;
		triplyPointerBuffer[0] = doublyPointerBufferOffset;
		inode->triplyPointer = triplyPointerBufferOffset;
		// write back
		fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider1 == 0) {
		getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		getAvailBlock(file, superBlock, groupDesc, &doublyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		doublyPointerBuffer[0] = singlyPointerBufferOffset;
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		triplyPointerBuffer[(inode->blockCount - bound2) / divider1] = doublyPointerBufferOffset;
		// write back
		fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider0 == 0) {
		getAvailBlock(file, superBlock, groupDesc, &singlyPointerBufferOffset);
		singlyPointerBuffer[0] = blockOffset;
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] = singlyPointerBufferOffset;
		// write back
		fseek(file, singlyPointerBufferOffset * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else if (inode->blockCount < bound3) {
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		singlyPointerBuffer[(inode->blockCount - bound2) % divider1 % divider0] = blockOffset;
		// write back
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
	}
	else
		return -1;
	// write back inode
	inode->blockCount ++;
	fseek(file, inodeOffset, SEEK_SET);
	fwrite((void*)inode, sizeof(Inode), 1, file);
	return 0;
}

/*
* to allocate 1 more block, 4 more blocks may be allocated in total in the worst case (i.e., 3 for pointer, and 1 for regular data
* allocBlock is safe, i.e., file, superBlock, groupDesc, inode, blockOffset won't be modified if allocBlock fails
*/
int allocBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
	int ret = 0;
	int blockOffset = 0;
	
	/* verify whether allocBlock would fail or not */
	ret = calNeededPointerBlocks(superBlock, inode->blockCount);
	if (superBlock->availBlockNum < ret + 1) // ret + 1 number of blocks should be allocated in total
		return -1;
	
	/* get the block to allocate */
	getAvailBlock(file, superBlock, groupDesc, &blockOffset);

	/* allocte pointer block of inode */
	allocLastBlock(file, superBlock, groupDesc, inode, inodeOffset, blockOffset);
	
	return 0;
}

/*
* free the last block of inode
*/
int freeLastBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
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
		setAllocBlock(file, superBlock, groupDesc, inode->pointer[inode->blockCount]);
	}
	else if (inode->blockCount == bound0) {
		fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, inode->singlyPointer);
	}
	else if (inode->blockCount < bound1) {
		fseek(file, inode->singlyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[inode->blockCount - bound0]);
	}
	else if (inode->blockCount == bound1) {
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, inode->doublyPointer);
	}
	else if (inode->blockCount < bound2 && (inode->blockCount - bound1) % divider0 == 0) {
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[(inode->blockCount - bound1) / divider0]);
	}
	else if (inode->blockCount < bound2) {
		fseek(file, inode->doublyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound1) / divider0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[(inode->blockCount - bound1) % divider0]);
	}
	else if (inode->blockCount == bound2) {
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, triplyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, inode->triplyPointer);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider1 == 0) {
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, triplyPointerBuffer[(inode->blockCount - bound2) / divider1]);
	}
	else if (inode->blockCount < bound3 && (inode->blockCount - bound2) % divider0 == 0) {
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[0]);
		setAllocBlock(file, superBlock, groupDesc, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0]);
	}
	else if (inode->blockCount < bound3) {
		fseek(file, inode->triplyPointer * SECTOR_SIZE, SEEK_SET);
		fread((void*)triplyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, triplyPointerBuffer[(inode->blockCount - bound2) / divider1] * SECTOR_SIZE, SEEK_SET);
		fread((void*)doublyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		fseek(file, doublyPointerBuffer[(inode->blockCount - bound2) % divider1 / divider0] * SECTOR_SIZE, SEEK_SET);
		fread((void*)singlyPointerBuffer, sizeof(uint8_t), superBlock->blockSize, file);
		setAllocBlock(file, superBlock, groupDesc, singlyPointerBuffer[(inode->blockCount - bound2) % divider1 % divider0]);
	}
	else
		return -1;
	// write back inode
	fseek(file, inodeOffset, SEEK_SET);
	fwrite((void*)inode, sizeof(Inode), 1, file);
	return 0;
}

/*
* to free all block of inode
*/
int freeBlock (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
	int ret = 0;
	while (inode->blockCount != 0) {
		ret = freeLastBlock(file, superBlock, groupDesc, inode, inodeOffset);
		if (ret == -1)
			return -1;
		//printf("FreeBlock.\n%d inodes and %d data blocks available.\n", superBlock->availInodeNum, superBlock->availBlockNum);
	}
	return 0;
}

int getDirEntry (FILE *file, SuperBlock *superBlock, Inode *inode, int dirIndex, DirEntry *destDirEntry) {
	int i = 0;
	int j = 0;
	int ret = 0;
	int dirCount = 0;
	DirEntry *dirEntry = NULL;
	uint8_t buffer[superBlock->blockSize];

	for (i = 0; i < inode->blockCount; i ++) {
		ret = readBlock(file, superBlock, inode, i, buffer);
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
int readInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc,
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
	fseek(file, *inodeOffset, SEEK_SET);
	fread((void*)destInode, sizeof(Inode), 1, file);

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
			ret = readBlock(file, superBlock, destInode, i, buffer);
			if (ret == -1)
				return -1;
			dirEntry = (DirEntry*)buffer;
			for (j = 0; j < superBlock->blockSize / sizeof(DirEntry); j ++) {
				if (dirEntry[j].inode == 0) // empty dirEntry
					continue;
				else if (stringCmp(dirEntry[j].name, destFilePath + count, size) == 0) {
					*inodeOffset = dirEntry[j].inode;
					fseek(file, *inodeOffset, SEEK_SET);
					fread((void*)destInode, sizeof(Inode), 1, file);
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
int getAvailInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int *inodeOffset) {
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
	fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fread((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file); // read inodeBitmap
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
		fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	}

	/* write inodeBitmap back */
	fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fwrite((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file); // write whole inodeBitmap

	return 0;
}

int setAllocInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc, int inodeOffset) {
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
	fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fread((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file);
	if ((inodeBitmap.byte[j] >> (7-k)) % 2 == 0)
		return -1;

	superBlock->availInodeNum ++;
	(groupDesc + i)->availInodeNum ++;
	inodeBitmap.byte[j] = inodeBitmap.byte[j] ^ (1 << (7-k));

	/* write superBlock back
	 *    write groupDesc back */
	for (i = 0; i < groupNum; i ++) {
		fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	}
	/* write inodeBitmap back */
	fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fwrite((void*)&inodeBitmap, sizeof(InodeBitmap), 1, file); // write whole blockBitmap
	
	return 0;
}

/*
* Supported destFilename pattern
* destFilename = "" empty string, echo error
*/
int allocInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc,
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
		ret = readBlock(file, superBlock, fatherInode, i, buffer);
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
		ret = allocBlock(file, superBlock, groupDesc, fatherInode, fatherInodeOffset);
		if (ret == -1)
			return -1;
		fatherInode->size = fatherInode->blockCount * superBlock->blockSize; // need to write back fatherInode
		setBuffer(buffer, superBlock->blockSize, 0); // reset buffer
		dirEntry = (DirEntry*)buffer;
		j = 0;
	}
	/* dirEntry[j] is the valid empty dirEntry, it is in the i-th block of fatherInode */
	/* get available inode */
	ret = getAvailInode(file, superBlock, groupDesc, destInodeOffset);
	if (ret == -1) // as we have check it in advance, it won't happen
		return -1;
	/* setting dirEntry */
	stringCpy(destFilename, dirEntry[j].name, NAME_LENGTH); // setting name of dirEntry
	dirEntry[j].inode = *destInodeOffset; // setting inode of dirEntry
	/* write back dirEntry */
	ret = writeBlock(file, superBlock, fatherInode, i, buffer); // write back i-th block of fatherInode
	if (ret == -1) // normally, it won't happen
		return -1;
	/* write back fatherInode */
	fseek(file, fatherInodeOffset, SEEK_SET);
	fwrite((void*)fatherInode, sizeof(Inode), 1, file);
	/* setting destInode */
	destInode->type = destFiletype;
	destInode->linkCount = 1;
	destInode->blockCount = 0;
	destInode->size = 0;
	/* write back destInode */
	fseek(file, *destInodeOffset, SEEK_SET);
	fwrite((void*)destInode, sizeof(Inode), 1, file);

	return 0;
}

int freeInode (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc,
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
		ret = readBlock(file, superBlock, fatherInode, i, buffer);
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
	fseek(file, *destInodeOffset, SEEK_SET);
	fread((void*)destInode, sizeof(Inode), 1, file);
	if (destInode->type != destFiletype) // verify the filetype
		return -1;
	if (destFiletype == DIRECTORY_TYPE) { // check if the directory is empty
		if (getDirEntry(file, superBlock, destInode, 0, &tmpDirEntry) != -1) // not empty
			return -1;
	}
	destInode->linkCount --;
	if (destInode->linkCount == 0) {
		/* setting inodeBitmap, groupDesc, superBlock
		 * write back inodeBitmap, groupDesc, superBlock */
		freeBlock(file, superBlock, groupDesc, destInode, *destInodeOffset);
		setAllocInode(file, superBlock, groupDesc, *destInodeOffset);
	}
	else {
		/* write back destInode */
		fseek(file, *destInodeOffset, SEEK_SET);
		fwrite((void*)destInode, sizeof(Inode), 1, file);
	}
	/* setting dirEntry[j] in i-th block of fatherInode*/
	dirEntry[j].inode = 0;
	/* write back i-th block of fatherInode */
	ret = writeBlock(file, superBlock, fatherInode, i, buffer);

	return 0;
}


int initRootDir (FILE *file, SuperBlock *superBlock, GroupDesc *groupDesc) {
	int i = 0;
	int groupNum = calGroupNum(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE);
	int groupSize = calGroupSize(superBlock->sectorNum, superBlock->blockSize / SECTOR_SIZE, groupNum, 0);
	int inodeBitmapOffset = 0;
	int inodeTableOffset = 0;
	int inodeOffset = 0;
	InodeBitmap inodeBitmap;
	Inode inode;
	
	/* check inode & block available or not */
	if (superBlock->availInodeNum == 0)
		return -1;
	superBlock->availInodeNum --;

	/* for a directory file, allocate a new inode */
	inodeBitmapOffset = groupDesc->inodeBitmap;
	inodeTableOffset = groupDesc->inodeTable;
	inodeBitmap.byte[0] = 0x80;
	inodeOffset = inodeTableOffset * SECTOR_SIZE; // byte as unit
	groupDesc->availInodeNum --;

	/* write superBlock back
	 * write groupDesc back */
	for (i = 0; i < groupNum; i ++) {
		fseek(file, i * groupSize * SECTOR_SIZE, SEEK_SET);
		fwrite((void*)superBlock, sizeof(SuperBlock), 1, file);
		fwrite((void*)groupDesc, sizeof(GroupDesc), groupNum, file);
	}

	/* write inodeBitmap back */
	fseek(file, inodeBitmapOffset * SECTOR_SIZE, SEEK_SET);
	fwrite((void*)&inodeBitmap, sizeof(uint8_t), 1, file); // write the 1st byte of inodeBitmap

	/* write inode back */
	inode.type = DIRECTORY_TYPE;
	inode.linkCount = 1;
	inode.blockCount = 0;
	inode.size = 0;
	fseek(file, inodeOffset, SEEK_SET);
	fwrite((void*)&inode, sizeof(Inode), 1, file); // write inode

	return 0;
}

int copyData (FILE *file, FILE *fileSrc, SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset) {
	int i = 0;
	int ret = 0;
	int size = 0;
	uint8_t buffer[superBlock->blockSize];
	fseek(fileSrc, 0, SEEK_SET);
	size = fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, fileSrc);

	while (size != 0) {
		if (i == inode->blockCount) {
			ret = allocBlock(file, superBlock, groupDesc, inode, inodeOffset);
			if (ret == -1) // no enough block to allocate
				return -1;
		}
		ret = writeBlock(file, superBlock, inode, i, buffer);
		if (ret == -1)
			return -1; // i is out of maximum range
		inode->size += size; // need to write back inode
		i ++;

		size = fread((void*)buffer, sizeof(uint8_t), superBlock->blockSize, fileSrc);
		//printf("CopyData.\n%d inodes and %d data blocks available.\n", superBlock->availInodeNum, superBlock->availBlockNum);
	}

	/* write back inode */
	fseek(file, inodeOffset, SEEK_SET);
	fwrite((void*)inode, sizeof(Inode), 1, file);
	return 0;
}

int format (const char *driver, int sectorNum, int sectorsPerBlock) {
	int i = 0;
	int ret = 0;
	FILE *file = NULL;
	uint8_t byte[SECTOR_SIZE];
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	if (driver == NULL) {
		printf("driver == NULL.\n");
		return -1;
	}
	file = fopen(driver, "w+"); // create / truncating / writing / reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	for (i = 0; i < SECTOR_SIZE; i++) {
		byte[i] = 0;
	}
	for (i = 0; i < sectorNum; i++) {
		fwrite((void*)byte, sizeof(uint8_t), SECTOR_SIZE, file);
	}
	ret = initGroupHeader(file, sectorNum, sectorsPerBlock, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to format: No enough sectors.\n");
		fclose(file);
		return -1;
	}
	ret = initRootDir(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to format: No enough inodes or data blocks.\n");
		fclose(file);
		return -1;
	}
	printf("format %s -s %d -b %d\n", driver, sectorNum, sectorsPerBlock);
	printf("FORMAT success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	return 0;
}

int mkdir (const char *driver, const char *destDirPath) {
	FILE *file = NULL;
	char tmp = 0;
	int length = 0;
	int cond = 0;
	int ret = 0;
	int size = 0;
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	int fatherInodeOffset = 0;
	int destInodeOffset = 0;
	Inode fatherInode;
	Inode destInode;
	if (driver == NULL) {
		printf("driver == NULL.\n");
		return -1;
	}
	file = fopen(driver, "r+"); // writing / reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	ret = readGroupHeader(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to load groupHeader.\n");
		fclose(file);
		return -1;
	}
	if (destDirPath == NULL) {
		printf("destDirPath == NULL");
		fclose(file);
		return -1;
	}
	length = stringLen(destDirPath);
	if (destDirPath[length - 1] == '/') { // last byte of destDirPath is '/'
		cond = 1;
		*((char*)destDirPath + length - 1) = 0;
	}
	ret = stringChrR(destDirPath, '/', &size);
	if (ret == -1) { // no '/' in destDirPath
		printf("Incorrect destination file path.\n");
		fclose(file);
		return -1;
	}
	tmp = *((char*)destDirPath + size +1);
	*((char*)destDirPath + size + 1) = 0;
	ret = readInode(file, &superBlock, groupDesc,
		&fatherInode, &fatherInodeOffset, destDirPath);
	*((char*)destDirPath + size + 1) = tmp;
	if (ret == -1) {
		printf("Failed to read father inode.\n");
		if (cond == 1)
			*((char*)destDirPath + length - 1) = '/';
		fclose(file);
		return -1;
	}
	ret = allocInode(file, &superBlock, groupDesc,
		&fatherInode, fatherInodeOffset,
		&destInode, &destInodeOffset, destDirPath + size + 1, DIRECTORY_TYPE);
	if (ret == -1) {
		printf("Failed to allocate inode.\n");
		if (cond == 1)
			*((char*)destDirPath + length - 1) = '/';
		fclose(file);
		return -1;
	}
	if (cond == 1)
		*((char*)destDirPath + length - 1) = '/';
	printf("mkdir %s\n", destDirPath);
	printf("MKDIR success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	return 0;
}

int rmdir (const char *driver, const char *destDirPath) {
	FILE *file = NULL;
	char tmp = 0;
	int length = 0;
	int cond = 0;
	int ret = 0;
	int size = 0;
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	int fatherInodeOffset = 0;
	int destInodeOffset = 0;
	Inode fatherInode;
	Inode destInode;
	if (driver == NULL) {
		printf("driver == NULL.\n");
		return -1;
	}
	file = fopen(driver, "r+"); // writing / reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	ret = readGroupHeader(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to load groupHeader.\n");
		fclose(file);
		return -1;
	}
	if (destDirPath == NULL) {
		printf("destDirPath == NULL");
		fclose(file);
		return -1;
	}
	length = stringLen(destDirPath);
	if (destDirPath[length - 1] == '/') { // last byte of destDirPath is '/'
		cond = 1;
		*((char*)destDirPath + length - 1) = 0;
	}
	ret = stringChrR(destDirPath, '/', &size);
	if (ret == -1) { // no '/' in destDirPath
		printf("Incorrect destination file path.\n");
		fclose(file);
		return -1;
	}
	tmp = *((char*)destDirPath + size +1);
	*((char*)destDirPath + size + 1) = 0;
	ret = readInode(file, &superBlock, groupDesc,
		&fatherInode, &fatherInodeOffset, destDirPath);
	*((char*)destDirPath + size + 1) = tmp;
	if (ret == -1) {
		printf("Failed to read father inode.\n");
		if (cond == 1)
			*((char*)destDirPath + length - 1) = '/';
		fclose(file);
		return -1;
	}
	ret = freeInode(file, &superBlock, groupDesc,
		&fatherInode, fatherInodeOffset,
		&destInode, &destInodeOffset, destDirPath + size + 1, DIRECTORY_TYPE);
	if (ret == -1) {
		printf("Failed to free inode and its block.\n");
		if (cond == 1)
			*((char*)destDirPath + length - 1) = '/';
		fclose(file);
		return -1;
	}
	printf("rmdir %s\n", destDirPath);
	printf("RMDIR success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	return 0;
}

int cp (const char *driver, const char *srcFilePath, const char *destFilePath) {
	FILE *file = NULL;
	FILE *fileSrc = NULL;
	char tmp = 0;
	int ret = 0;
	int size = 0;
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	int fatherInodeOffset = 0;
	int destInodeOffset = 0;
	Inode fatherInode;
	Inode destInode;
	if (driver == NULL || srcFilePath == NULL) {
		printf("driver == NULL || srcFilePath == NULL.\n");
		return -1;
	}
	file = fopen(driver, "r+"); // writing / reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	fileSrc = fopen(srcFilePath, "r"); // reading
	if (fileSrc == NULL) {
		printf("Failed to open srcFilePath.\n");
		fclose(file);
		return -1;
	}
	ret = readGroupHeader(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to load groupHeader.\n");
		fclose(file);
		fclose(fileSrc);
		return -1;
	}
	if (destFilePath == NULL) {
		printf("destFilePath == NULL.\n");
		fclose(file);
		fclose(fileSrc);
		return -1;
	}
	ret = stringChrR(destFilePath, '/', &size);
	if (ret == -1) { // no '/' in destFilePath
		printf("Incorrect destination file path.\n");
		fclose(file);
		fclose(fileSrc);
		return -1;
	}
	tmp = *((char*)destFilePath + size + 1);
	*((char*)destFilePath + size + 1) = 0;
	ret = readInode(file, &superBlock, groupDesc,
		&fatherInode, &fatherInodeOffset, destFilePath);
	*((char*)destFilePath + size + 1) = tmp;
	if (ret == -1) {
		printf("Failed to read father inode.\n");
		fclose(file);
		fclose(fileSrc);
		return -1;
	}
	ret = allocInode(file, &superBlock, groupDesc, // safe operation, none of the parameters would be modified if it fails
		&fatherInode, fatherInodeOffset,
		&destInode, &destInodeOffset, destFilePath + size + 1, REGULAR_TYPE);
	if (ret == -1) {
		printf("Failed to allocate inode.\n");
		fclose(file);
		fclose(fileSrc);
		return -1;
	}
	ret = copyData(file, fileSrc, &superBlock, groupDesc, &destInode, destInodeOffset); // unsafe operation, file, superBlock, groupDesc, destInode would be modified
	if (ret == -1) {
		printf("Failed to copy data.\n");
		fclose(file);
		fclose(fileSrc);
		return -1;
	}
	printf("cp %s %s\n", srcFilePath, destFilePath);
	printf("CP success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	fclose(fileSrc);
	return 0;
}

int rm (const char *driver, const char *destFilePath) {
	FILE *file = NULL;
	char tmp = 0;
	int ret = 0;
	int size = 0;
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	int fatherInodeOffset = 0;
	int destInodeOffset = 0;
	Inode fatherInode;
	Inode destInode;
	if (driver == NULL) {
		printf("driver == NULL.\n");
		return -1;
	}
	file = fopen(driver, "r+"); // writing / reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	ret = readGroupHeader(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to load groupHeader.\n");
		fclose(file);
		return -1;
	}
	if (destFilePath == NULL) {
		printf("destFilePath == NULL.\n");
		fclose(file);
		return -1;
	}
	ret = stringChrR(destFilePath, '/', &size);
	if (ret == -1) { // no '/' in destFilePath
		printf("Incorrect destination file path.\n");
		fclose(file);
		return -1;
	}
	tmp = *((char*)destFilePath + size + 1);
	*((char*)destFilePath + size + 1) = 0;
	ret = readInode(file, &superBlock, groupDesc,
		&fatherInode, &fatherInodeOffset, destFilePath);
	*((char*)destFilePath + size + 1) = tmp;
	if (ret == -1) {
		printf("Failed to read father inode.\n");
		fclose(file);
		return -1;
	}
	ret = freeInode(file, &superBlock, groupDesc,
		&fatherInode, fatherInodeOffset,
		&destInode, &destInodeOffset, destFilePath + size + 1, REGULAR_TYPE);
	if (ret == -1) {
		printf("Failed to free inode and its block.\n");
		fclose(file);
		return -1;
	}
	printf("rm %s\n", destFilePath);
	printf("RM success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	return 0;
}

int ls (const char *driver, const char *destFilePath) {
	FILE *file = NULL;
	int ret = 0;
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	int inodeOffset = 0;
	Inode inode;
	Inode childInode;
	int dirIndex = 0;
	DirEntry dirEntry;
	if (driver == NULL) {
		printf("driver == NULL.\n");
		return -1;
	}
	file = fopen(driver , "r"); // reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	ret = readGroupHeader(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to load groupHeader.\n");
		fclose(file);
		return -1;
	}
	if (destFilePath == NULL) {
		printf("destFilePath == NULL.\n");
		fclose(file);
		return -1;
	}
	ret = readInode(file, &superBlock, groupDesc,
		&inode, &inodeOffset, destFilePath);
	if (ret == -1) {
		printf("Failed to read inode.\n");
		fclose(file);
		return -1;
	}
	if (inode.type == REGULAR_TYPE) {
		printf("ls %s\n", destFilePath);
		printf("Type: %d, LinkCount: %d, BlockCount: %d, Size: %d.\n",
			inode.type, inode.linkCount, inode.blockCount, inode.size);
		fclose(file);
		return 0;
	}
	printf("ls %s\n", destFilePath);
	while (getDirEntry(file, &superBlock, &inode, dirIndex, &dirEntry) == 0) {
		dirIndex ++;
		fseek(file, dirEntry.inode, SEEK_SET);
		fread((void*)&childInode, sizeof(Inode), 1, file);
		printf("Name: %s, Type: %d, LinkCount: %d, BlockCount: %d, Size: %d.\n",
			dirEntry.name, childInode.type, childInode.linkCount, childInode.blockCount, childInode.size);
	}
	printf("LS success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	return 0;
}

int cat (const char *driver, const char *destFilePath) {
	FILE *file = NULL;
	int i = 0;
	int ret = 0;
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	int inodeOffset = 0;
	Inode inode;
	uint8_t buffer[SECTOR_SIZE * SECTORS_PER_BLOCK];
	if (driver == NULL) {
		printf("driver == NULL.\n");
		return -1;
	}
	file = fopen(driver , "r"); // reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	ret = readGroupHeader(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to load groupHeader.\n");
		fclose(file);
		return -1;
	}
	if (destFilePath == NULL) {
		printf("destFilePath == NULL.\n");
		fclose(file);
		return -1;
	}
	ret = readInode(file, &superBlock, groupDesc,
		&inode, &inodeOffset, destFilePath);
	if (ret == -1) {
		printf("Failed to read inode.\n");
		fclose(file);
		return -1;
	}
	if (inode.type == DIRECTORY_TYPE) {
		printf("cat %s: is a directory.\n", destFilePath);
		fclose(file);
		return -1;
	}
	printf("cat %s\n", destFilePath);
	for (i = 0; i < inode.blockCount; i ++) {
		readBlock(file, &superBlock, &inode, i, buffer);
		printf("%s", (char*)buffer);
	}
	printf("\nCAT success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	return 0;
}

int touch (const char *driver, const char *destFilePath) {
	FILE *file = NULL;
	char tmp = 0;
	int ret = 0;
	int size = 0;
	SuperBlock superBlock;
	GroupDesc groupDesc[MAX_GROUP_NUM];
	int fatherInodeOffset = 0;
	int destInodeOffset = 0;
	Inode fatherInode;
	Inode destInode;
	if (driver == NULL) {
		printf("driver == NULL.\n");
		return -1;
	}
	file = fopen(driver, "r+"); // writing / reading
	if (file == NULL) {
		printf("Failed to open driver.\n");
		return -1;
	}
	ret = readGroupHeader(file, &superBlock, groupDesc);
	if (ret == -1) {
		printf("Failed to load groupHeader.\n");
		fclose(file);
		return -1;
	}
	if (destFilePath == NULL) {
		printf("destFilePath == NULL.\n");
		fclose(file);
		return -1;
	}
	ret = stringChrR(destFilePath, '/', &size);
	if (ret == -1) { // no '/' in destFilePath
		printf("Incorrect destination file path.\n");
		fclose(file);
		return -1;
	}
	tmp = *((char*)destFilePath + size + 1);
	*((char*)destFilePath + size + 1) = 0;
	ret = readInode(file, &superBlock, groupDesc,
		&fatherInode, &fatherInodeOffset, destFilePath);
	*((char*)destFilePath + size + 1) = tmp;
	if (ret == -1) {
		printf("Failed to read father inode.\n");
		fclose(file);
		return -1;
	}
	ret = allocInode(file, &superBlock, groupDesc, // safe operation, none of the parameters would be modified if it fails
		&fatherInode, fatherInodeOffset,
		&destInode, &destInodeOffset, destFilePath + size + 1, REGULAR_TYPE);
	if (ret == -1) {
		printf("Failed to allocate inode.\n");
		fclose(file);
		return -1;
	}
	printf("touch %s\n", destFilePath);
	printf("TOUCH success.\n%d inodes and %d data blocks available.\n", superBlock.availInodeNum, superBlock.availBlockNum);
	fclose(file);
	return 0;
}
