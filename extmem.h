/*
 * extmem.h
 * Zhaonian Zou
 * Harbin Institute of Technology
 * Jun 22, 2011
 */

#ifndef EXTMEM_H
#define EXTMEM_H

#define BLOCK_AVAILABLE 0
#define BLOCK_UNAVAILABLE 1


typedef struct tagBuffer {
    unsigned long numIO; /* Number of IO's*/
    size_t bufSize; /* Buffer size*/
    size_t blkSize; /* Block size */
    size_t numAllBlk; /* Number of blocks that can be kept in the buffer */
    size_t numFreeBlk; /* Number of available blocks in the buffer */
    unsigned char *data; /* Starting address of the buffer */
} Buffer;


/**
 * 初始化缓冲区buffer
 * @param bufSize
 * @param blkSize
 * @param buf
 * @return  If the initialization fails, the return value is NULL; otherwise the pointer to the buffer.
 */
Buffer *initBuffer(size_t bufSize, size_t blkSize, Buffer *buf);

/**
 * 释放缓冲区buf占用的内存空间
 * @param buf
 */
void freeBuffer(Buffer *buf);

/**
 * 在缓冲区中申请一个新的块
 * @param buf
 * @return If no free blocks are available in the buffer,then the return value is NULL; otherwise the pointer to the block.
 */
unsigned char *getNewBlockInBuffer(Buffer *buf);

/**
 * 解除块对缓冲区内存的占用
 * @param blk
 * @param buf
 */
void freeBlockInBuffer(unsigned char *blk, Buffer *buf);

/**
 * 从磁盘上删除地址为addr的磁盘块内的数据
 * Drop a block on the disk
 * @param addr
 * @return
 */
int dropBlockOnDisk(unsigned int addr);

/**
 * 将磁盘上地址为addr的磁盘块读入缓冲区buf
 * @param addr
 * @param buf
 * @return
 */
unsigned char *readBlockFromDisk(unsigned int addr, Buffer *buf);


/**
 * 将缓冲区buf内的块blk写入磁盘上地址为addr的磁盘块
 * @param addr
 * @param buf
 * @return
 */
int writeBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf);

#endif // EXTMEM_H
