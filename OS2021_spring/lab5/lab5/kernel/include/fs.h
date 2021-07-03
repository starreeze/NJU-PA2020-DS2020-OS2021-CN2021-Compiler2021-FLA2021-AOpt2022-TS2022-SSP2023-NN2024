#ifndef __FS_H__
#define __FS_H__

#include "fs/ext.h"

int readGroupHeader (SuperBlock *superBlock, GroupDesc *groupDesc);

int allocInode (SuperBlock *superBlock, GroupDesc *groupDesc,
		Inode *fatherInode, int fatherInodeOffset,
		Inode *destInode, int *destInodeOffset, const char *destFilename, int destFiletype);
int freeInode (SuperBlock *superBlock, GroupDesc *groupDesc,
		Inode *fatherInode, int fatherInodeOffset,
		Inode *destInode, int *destInodeOffset, const char *destFilename, int destFiletype);
	
int readInode (SuperBlock *superBlock, GroupDesc *groupDesc,
		Inode *destInode, int *inodeOffset, const char *destFilePath);

int allocBlock (SuperBlock *superBlock, GroupDesc *groupDesc, Inode *inode, int inodeOffset);

int readBlock (SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer);

int writeBlock (SuperBlock *superBlock, Inode *inode, int blockIndex, uint8_t *buffer);

int getDirEntry (SuperBlock *superBlock, Inode *inode, int dirIndex, DirEntry *destDirEntry);

void initFS (void);

void initFile (void);

#endif
