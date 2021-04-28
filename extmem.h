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
 * ��ʼ��������buffer
 * @param bufSize
 * @param blkSize
 * @param buf
 * @return  If the initialization fails, the return value is NULL; otherwise the pointer to the buffer.
 */
Buffer *initBuffer(size_t bufSize, size_t blkSize, Buffer *buf);

/**
 * �ͷŻ�����bufռ�õ��ڴ�ռ�
 * @param buf
 */
void freeBuffer(Buffer *buf);

/**
 * �ڻ�����������һ���µĿ�
 * @param buf
 * @return If no free blocks are available in the buffer,then the return value is NULL; otherwise the pointer to the block.
 */
unsigned char *getNewBlockInBuffer(Buffer *buf);

/**
 * �����Ի������ڴ��ռ��
 * @param blk
 * @param buf
 */
void freeBlockInBuffer(unsigned char *blk, Buffer *buf);

/**
 * �Ӵ�����ɾ����ַΪaddr�Ĵ��̿��ڵ�����
 * Drop a block on the disk
 * @param addr
 * @return
 */
int dropBlockOnDisk(unsigned int addr);

/**
 * �������ϵ�ַΪaddr�Ĵ��̿���뻺����buf
 * @param addr
 * @param buf
 * @return
 */
unsigned char *readBlockFromDisk(unsigned int addr, Buffer *buf);


/**
 * ��������buf�ڵĿ�blkд������ϵ�ַΪaddr�Ĵ��̿�
 * @param addr
 * @param buf
 * @return
 */
int writeBlockToDisk(unsigned char *blkPtr, unsigned int addr, Buffer *buf);

#endif // EXTMEM_H
