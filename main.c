#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include <string.h>
/*
 * 实验要求：使用有限内存（ Buffer ）实现上述算法，不可定义长度大于10的整型或字符型数组。
 * 关系R具有两个属性A和B， A的值域为[20, 60]，B的值域为[1000, 2000]；
 * 关系S具有两个属性C和D，C的值域为[40, 80]，D的值域为[1000, 3000]。
 * 属性值均为int型（4个字节），R和S的每个元组的大小均为8个字节。
 *  1. 题目中的所有选择、排序、连接， 都是基于关系的第一个属性，即R.A 和 S.C。
    2. 实验数据经过修改，两个关系一共有15个相同的元组。为方便大家调试，给出其中5个相同的元组：(40,1243),(41,1829),(42,1422),(58,1256),(58,1747)。
    3. 实验中只可以使用内存缓冲区（520字节）排序，排序中发生的交换要Buffer中进行， 不可定义size>10的数组。
    4. 排序只能在Buffer内，也不可定义多个size<10的整型数组来排序。
    5. 集合的并、交、差是独立的三个算法， 并和差操作不可以利用交集的结果。
    6. 随机生成的关系S中存在相同元组，请大家替换data目录下的25.blk为新上传的文件。

 */
/**
 * 自定义字符串转数字，保证不会发生数组越界产生的副作用（放弃字符串最后的结束表示’\0‘）
 * @param str 需要转换的字符串
 * @param length  转换的长度
 * @return 转换结果
 */
int myAtoi(char *str, int length);

/**
 * 数字转字符串
 * @param n 需要转换的数字
 * @param s  放置该数字的字符串
 */
void myItoa(int, char *s);

/**
 * 展示用户界面
 * @return 用户选择的指令
 */
int ShowUserInterface();

/**
 * 使用三异或的原理对address1和address2对应地址中长度为length的内容进行交换
 * @param address1  地址1
 * @param address2  地址2
 * @param length  交换的长度
 */
void swap(unsigned char *address1, unsigned char *address2, int length);

/**
 * 将数据块blk写入磁盘
 * @param blk 数据块
 * @param buf 缓冲区
 * @param address 写入的地址
 */
void saveBlock(unsigned char *blk, Buffer *buf, int address);

/**
 * 从块blk 中读取位于address+offset位置的字符并转换成整形数字
 * @param blk 目标块
 * @param address 字符开始的地址 通常是8的倍数
 * @param offset  偏移量 通常是0或4
 * @return address+offset位置的字符对应的整形数字
 */
int readNumberFromBlk(unsigned char *blk, int address, int offset);

/**
 * 实现基于线性搜索的关系选择算法：基于ExtMem程序库，使用C语言实现线性搜索算法，选出S.C=50的元组，记录IO读写次数，并将选择结果存放在磁盘上。
 * （模拟实现 select S.C,S.D from S where S.C = 50）

 * @param buf 缓冲区
 * @param value 搜索的数值
 */
void LinearSearch(Buffer *buf, int value);

/**
 * 首先划分子集并子集排序
 * DivideSort()函数对数据库的块进行子集划分，并使用冒泡排序在子集中对数据元组进行排序，并将结果储存到数据块中
 * @param buf 缓冲区
 * @param begin 子集在初始块数据的起始位置
 * @param end 子集在初始块数据的起终止位置
 */
void DivideSort(Buffer *buf, int begin, int end);

/**
  (二) 各子集间归并排序
   各分组依次读取一个元素形成待比较集合，将待比较集合中的最小元素写入输出块
   原理就是分别读取每一个子集中最小的数值（因为子集已经被排序过了，其实就是读取子集中最靠前的数值），然后在不同子集之间进行比较，选择输出最小的，直到所有的子集中的数值都输出完即可
 * @param buf 缓冲区
 * @param begin 子集在初始块数据的起始位置
 * @param end 子集在初始块数据的起终止位置
 * @param outputBlkNum 输出块的数目
 */
void MergeSort(Buffer *buf, int begin, int end, int outputBlkNum);

/**
 * 两阶段多路归并排序算法
 * @param buf 缓冲区
 * @param source  选择的关系
 */
void TPMMS(Buffer *buf, char source);

/**
 * 生成关系S的索引文件
 * 建立索引的过程依赖于第二问的排序结果，具体流程就是从小到大遍历排序结果，对出现的每一个不同的索引字段记录其第一次出现的磁盘块，并将该信息与索引字段信息一同记录到索引文件中
 * @param buf 缓冲区
 */
void CreateIndex(Buffer *buf);

/**
 *  实现基于索引的关系选择算法：利用（2）中的排序结果为关系S建立索引文件，利用索引文
    件选出S.C=50的元组，并将选择结果存放在磁盘上。记录IO读写次数，与（1）中的结果对
比。（模拟实现 select S.C, S.D from S where S.C = 50 ）
   在建立索引后，即可基于索引进行搜索,首先读取索引文件，根据索引字段获取其所储存的区块的位置（targetBlkNo），
   然后再在该区块中查找全部符合条件的数值，因为区块是已经排好序的，因此如果在索引字段对应的数据的数目较大就会分布在多个区块中，遍历完一个区块则会在下一个区块中继续搜索，直到遇到不同的索引字段，搜索终止，最后将搜索结果持久化到磁盘块即可
   如果没有发现索引字段，则说明搜索没有产生结果
 * @param buf 缓冲区
 * @param value搜素的数值
 */
void IndexSearch(Buffer *buf, int value);

/**
 *   实现基于排序的连接操作算法（Sort-Merge-Join）：对关系S和R计算S.C连接R.A ，并统计
    连接次数，将连接结果存放在磁盘上。 （模拟实现 select S.C, S.D, R.A, R.B from S inner
    join R on S.C = R.A）
 */
void SortMergeJoin(Buffer *buf);

/**
 * 对分块排序后的结果基于并操作进行归并
    在排序后执行归并阶段，具体流程就是对于S和R两组关系在每一组中一次选出最小的数值，
    然后两者（sc与ra）进行比较，如果不相同的话则弹出两者中的较小值，然后输出较小者，如果两者相同，再判断rb与sd，如果存在小于关系的话就输出较小的值对应的元组
   如果两者相同的话根据并操作，只输出一个元组即可
 * @param buf 缓冲区
 */
void MergeUnion(Buffer *buf);

/**
 * 对分块排序后的结果基于交操作进行归并
 * 在排序后执行归并阶段，具体流程就是对于S和R两组关系在每一组中一次选出最小的数值，
    然后两者（sc与ra）进行比较，如果不相同的话则弹出两者中的较小值，然后跳过较小者，如果两者相同，再判断rb与sd，如果还是相同的话则输出交操作的结果，否则跳过较小的元组继续比较
 * @param buf 缓冲区
 */
void MergeIntersect(Buffer *buf);

/**
 * 对分块排序后的结果基于差操作进行归并
 * 在排序后执行归并阶段，具体流程就是对于S和R两组关系在每一组中一次选出最小的数值，
    然后两者（sc与ra）进行比较，如果不相同的话则弹出两者中的较小值，如果较小者是sc的话则输出sc，否则跳过（不输出）ra，如果两者相同，再判断rb与sd，如果还是相同的话则跳过该sc，直到sc与ra都归并完成
 * @param buf 缓冲区
 */
void MergeExcept(Buffer *buf);

/**
 * 对分块排序后的结果基于连接操作进行归并
 * 在排序后执行归并阶段，具体流程就是对于S和R两组关系在每一组中一次选出最小的数值，然后两者（sc与ra）进行比较，如果不相同的话则弹出两者中的较小值，
   直到两者相同，然后再进行连接，就是以sc为基准，遍历两路中所有与sc相等的ra，并将两个元组一起写入输出块中
 * @param buf 缓冲区
 */
void MergeJoin(Buffer *buf);


 /**
  * 实现基于排序或散列的两趟扫描算法，实现并（S R） 、交（S R） 、差（S - R）其中
一种集合操作算法。将结果存放在磁盘上，并统计并、交、差操作后的元组个数。
 并运算是将两个关系的元组合并成一个关系，在合并时去掉重复的元组
  * @param buf 缓冲区
  */
void Union(Buffer *buf) {
    MergeUnion(buf);
}
/**
 * 实现基于排序或散列的两趟扫描算法，实现并（S R） 、交（S R） 、差（S - R）其中
一种集合操作算法。将结果存放在磁盘上，并统计并、交、差操作后的元组个数。
并运算是将两个关系的元组合并成一个关系，在合并时去掉重复的元组
 * @param buf 缓冲区
 */
void Intersect(Buffer *buf) {
    MergeIntersect(buf);
}
/**
 * 实现基于排序或散列的两趟扫描算法，实现并（S R） 、交（S R） 、差（S - R）其中
一种集合操作算法。将结果存放在磁盘上，并统计并、交、差操作后的元组个数。
并运算是将两个关系的元组合并成一个关系，在合并时去掉重复的元组
 * @param buf 缓冲区
 */
void Except(Buffer *buf) {
    MergeExcept(buf);
}
/**
 * 主函数，没什么好说的
 * @param argc 参数数目
 * @param argv  参数数组
 * @return  正常运行的话返回0，否则会返回异常值
 */
int main(int argc, char **argv) {
    Buffer buf; /* A buffer */
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int input = 4;
    input = ShowUserInterface();
    int value = 50;
    if (input == 1) {
        LinearSearch(&buf, value);
    } else if (input == 2) {
        TPMMS(&buf, 'S');
        TPMMS(&buf, 'R');

    } else if (input == 3) {
        IndexSearch(&buf, value);
    } else if (input == 4) {
        SortMergeJoin(&buf);
    } else if (input == 5) {
        Union(&buf);
    } else if (input == 6) {
        Intersect(&buf);
    } else if (input == 7) {
        Except(&buf);
    } else {
        printf("请输入正确的指令\n");
    }
    printf("IO's is %lu\n", buf.numIO); /* Check the number of IO's */
    return 0;
}

void LinearSearch(Buffer *buf, int value) {
    int i, j;
    int flag = 0;
    unsigned char *r_blk; //读取缓冲区
    unsigned char *w_blk = getNewBlockInBuffer(buf); //输出缓冲区
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int next_blk = 1;
    for (i = 17; i <= 48; i++) {
        printf("读入数据块：%d.blk\n", i);
        if ((r_blk = readBlockFromDisk(i, buf)) == NULL) {//读取磁盘块
            perror("Reading Block Failed!\n");
            return;
        }
        for (j = 0; j < 7; j++) {//一个块可以装7组数据
            int int_C = readNumberFromBlk(r_blk, 8 * j, 0);//将磁盘块中的C转为int型
            int int_D = readNumberFromBlk(r_blk, 8 * j, 4);//将磁盘块中的D转为int型
            if (int_C == value) {//判断数值是否匹配
                printf("%d:(S.C=%d,S.D=%d)\n", flag, int_C, int_D);
                flag++;
                if (next_blk == 1) {//如果输出缓冲区w_blk还没有写过
                    next_blk++; //下一块是102
                }
                if (write_cnt == 7) {//如果一个块已经写满了，就将这个快用saveBlock()函数进行提交
                    saveBlock(w_blk, buf, 1000 + next_blk - 1);//块编号为1000+x（x为当前写入块的序号）
                    w_blk = getNewBlockInBuffer(buf);//提交后获取一个新的块
                    memset(w_blk, 0, 64);
                    write_cnt = 0;//重置write_cnt为0，便于继续技术
                    next_blk++;
                }
                memcpy(w_blk + write_cnt * 8, r_blk + 8 * j, 4);//将r_blk中的s.c写入输出块 write_cnt * 8的地址
                memcpy(w_blk + write_cnt * 8 + 4, r_blk + 8 * j + 4, 4);//将r_blk中的s.d写入输出块 write_cnt * 8 +4的地址
                write_cnt++;
            }
        }
        freeBlockInBuffer((unsigned char *) r_blk, buf);//遍历过一个磁盘块后，将其占用的buffer释放掉

    }
    if (flag == 0) {
        printf("未发现任何结果满足 S.C=%d\n", value);
    } else {
        saveBlock(w_blk, buf, 1000 + next_blk - 1);//提交循环结束前还未提交的最后一个块
        printf("发现 %d 条结果满足 S.C=%d\n结果已写入文件 %d.blk---%d.blk\n", flag, value, 1001, next_blk + 1000 - 1);
        return;
    }

}

void DivideSort(Buffer *buf, int begin, int end) {
    unsigned char *blk_array[buf->numAllBlk];//使用所有的缓冲区，用于储存中间结果
    //待排序数据不能一次装入内存，需将数据分批装入分批处理。

    for (int outer = begin; outer <= end; outer += 8) {//遍历所有的块
        int inner = 0;
        for (inner = 0; inner < 8; inner++) {

            if ((blk_array[inner] = readBlockFromDisk(outer + inner, buf)) == NULL) {
                perror("Reading Block Failed!\n");
                return;
            }
        }
        printf("已读取:%d.blk->.%d.blk到缓冲区\n", outer, outer + inner - 1);
        //printf("在缓冲区内执行内排序\n");

        //一个缓冲区最多装8块数据，每块中包含7个元组，一个元组包含两个数据，按照第一条排序
        int i, j, k, l;
        //实现一个冒泡排序  外循环从 0到 len-1 内循环从 0到 len-1-i
        for (i = 0; i < inner; i++) {//外层循环   读取缓冲区每个块
            for (j = 0; i != inner - 1 ? j < 7 : j < 6; j++) { //读取每块中包含7个元组
                int cnt = 56 - 1 - (i * 7 + j);
                for (k = 0; k <= cnt / 7; k++) {
                    for (l = 0; k == cnt / 7 ? l < cnt % 7 : l < 7; l++) {
                        //printf("%d*7+%d=%d\n",k,l,k*7+l);
                        int a_one = readNumberFromBlk(blk_array[k], 8 * l, 0);
                        int a_two = readNumberFromBlk(blk_array[k], 8 * l, 4);
                        int b_one = -1;
                        int b_two = -1;

                        //两个待交换位置的元组如果跨块储存,就从下一块的开头读取
                        if (l == 6 && k != inner - 1) {
                            b_one = readNumberFromBlk(blk_array[k + 1], 0, 0);
                            b_two = readNumberFromBlk(blk_array[k + 1], 0, 4);

                        } else {//处于同一个块
                            b_one = readNumberFromBlk(blk_array[k], 8 * (l + 1), 0);
                            b_two = readNumberFromBlk(blk_array[k], 8 * (l + 1), 4);

                        }

                        if (a_one > b_one || (a_one == b_one && a_two > b_two)) { //交换顺序
                            unsigned char *address1 = blk_array[k] + 8 * l;
                            unsigned char *address2;
                            if (l == 6 && k != inner - 1) {//分别处于两个块时
                                address2 = blk_array[k + 1];
                            } else {//处于同一个块
                                address2 = blk_array[k] + 8 * (l + 1);
                            }
                            swap(address1, address2, 8);//使用三异或的swap()函数
                        }
                    }
                }
            }
        }
        //冒泡排序后写回2000+原本的序号位置
        for (inner = 0; inner < 8; inner++) {
            saveBlock(blk_array[inner], buf, outer + inner + 2000);
        }
    }
}

void MergeSort(Buffer *buf, int begin, int end, int outputBlkNum) {

    unsigned char *in_blk_array[outputBlkNum];//读取的区块的数组
    unsigned char *out_blk = getNewBlockInBuffer(buf);//输出块，大小为1
    memset(out_blk, 0, 64);
    int write_cnt = 0; //输出的计数器
    int number[outputBlkNum];//储存比较的队列
    int left[outputBlkNum];//左总游标，范围[0,55]
    int leftInner[outputBlkNum];//左块内部游标，范围[0,6],
    int right[outputBlkNum];//右游标,都是7*8=56
    for (int i = 0; i < outputBlkNum; i++) {//首先对划分后的每一个子集进行初始化
        left[i] = 0; //初始化左总游标
        leftInner[i] = 0;
        right[i] = 8 * 7;//每个组有8个块
        if ((in_blk_array[i] = readBlockFromDisk(2000 + i * 8 + begin, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    int flag = 0;//flag用与判断归并的终止条件
    while (flag == 0) {
        for (int i = 0; i < outputBlkNum; i++) {
            if (left[i] == right[i])//当前组已经处理完，直接跳过就行
                continue;

            if (leftInner[i] >= 7) {//已经读取完了一个块
                leftInner[i] = 0;
                freeBlockInBuffer(in_blk_array[i], buf);//释放之前的块
                if ((in_blk_array[i] = readBlockFromDisk(2000 + left[i] / 7 + i * 8 + begin, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            number[i] = readNumberFromBlk(in_blk_array[i], leftInner[i] * 8, 0);
        }
        int min = 1 << 30, minIndex = -1;
        for (int i = 0; i < outputBlkNum; i++) {//选出最小的数和对应组的编号
            if (left[i] == right[i])
                continue;

            min = number[i] < min ? number[i] : min;
            minIndex = number[i] == min ? i : minIndex;
        }
        //printf("minIndex=%d num=%d\n",minIndex,min);
        //处理最小值
        if (write_cnt % 7 != 0 || write_cnt == 0) {//一个输出块写满了，就将其提交并申请新的块
            memcpy(out_blk + (write_cnt % 7) * 8, in_blk_array[minIndex] + leftInner[minIndex] * 8, 8);
        } else {
            saveBlock(out_blk, buf, 2100 + write_cnt / 7 + begin - 1);
            out_blk = getNewBlockInBuffer(buf);
            memset(out_blk, 0, 64);
            memcpy(out_blk + (write_cnt % 7) * 8, in_blk_array[minIndex] + leftInner[minIndex] * 8, 8);

        }
        write_cnt++;
        left[minIndex]++;
        leftInner[minIndex]++;
        //printf("write_cnt=%d \n",write_cnt);
        for (int i = 0; i < outputBlkNum; i++) {//判断是否全部读取完，即检查所有的左游标和右游标是否相等，相等即是读取完成
            if (left[i] != right[i]) {
                //printf("i=%d,left[i]=%d\n",i,left[i]);
                flag = 0;
                break;
            } else {
                flag = 1;
            }
        }
    }
    saveBlock(out_blk, buf, 2100 + (write_cnt - 1) / 7 + 1 + begin - 1);//将最后的区块保存2100+write_cnt/7(向上取整)+起始位置
    for (int i = 0; i < outputBlkNum; i++) {//释放全部子集用于读入的的buffer
        freeBlockInBuffer(in_blk_array[i], buf);
    }
}

void TPMMS(Buffer *buf, char source) {
    int begin, end, outputBlkNum;//设置数据库的起点和终点,循环计数器

    switch (source) { //根据数据库选择进行赋值
        case 'S':
            begin = 17;
            end = 48;
            outputBlkNum = 4;
            break;
        case 'R':
            begin = 1;
            end = 16;
            outputBlkNum = 2;
            break;
        default:
            perror("选择的数据库错误");
            return;
    }
    DivideSort(buf, begin, end);
    MergeSort(buf, begin, end, outputBlkNum);
    printf("关系%c的排序已完成,输出存放在 %d.blk->%d.blk\n", source, 2100 + begin, 2100 + end);

}

void CreateIndex(Buffer *buf) {
    unsigned char *r_blk; //使用一个缓冲区的块进行读入
    unsigned char *w_blk = getNewBlockInBuffer(buf); /* A pointer to a block */
    memset(w_blk, 0, 64);
    int write_cnt = 0;//写操作的计数器
    int last_value = 1 << 31, cur_value = 0;//last_value维护当前访问过的最小值，cur_value则保存当前正在访问的元组的数值
    int begin = 17, end = 48;//块的开始和结束
    for (int i = begin; i <= end; i++) {
        if ((r_blk = readBlockFromDisk(i + 2100, buf)) == NULL) {//读取排序结果
            perror("Reading Sorted Block Failed!\n");
            return;
        }
        for (int j = 0; j < 7; j++) {
            cur_value = readNumberFromBlk(r_blk, 8 * j, 0);//记录当前索引字段的值
            //printf("cur=%d,last=%d\n",cur_value,last_value);
            if (cur_value > last_value) {//如果出现新的索引字段（比之前任何索引字段都要大）
                char string_blkNo[4];//装载当前块号信息
                myItoa(i + 2100, string_blkNo);//将块号转化为字符串可u需变更
                if (write_cnt % 7 != 0 || write_cnt == 0) {
                    memcpy(w_blk + (write_cnt % 7) * 8, r_blk + 8 * j, 4);//将索引的字段写入输出块中 (write_cnt % 7) * 8的位置
                    memcpy(w_blk + (write_cnt % 7) * 8 + 4, string_blkNo, 4);//将索引的字段对应第一次数显的块编号写入输出块中 (write_cnt % 7) * 8的位置

                } else {//一个块写满了，将其写回到文件 3000+序号.blk
                    saveBlock(w_blk, buf, 3000 + write_cnt / 7 + begin - 1);
                    w_blk = getNewBlockInBuffer(buf);
                    memset(w_blk, 0, 64);
                    memcpy(w_blk, r_blk + 8 * j, 4);
                    memcpy(w_blk + 4, string_blkNo, 4);

                }
                write_cnt++;
                last_value = cur_value;//更新记录
            }
        }
        freeBlockInBuffer(r_blk, buf);//释放读取的缓存
    }
    if (write_cnt % 7 != 0) //如果遍历以变后写入缓冲区还有数值没写
        saveBlock(w_blk, buf, 3000 + write_cnt / 7 + begin);

}

void IndexSearch(Buffer *buf, int value) {
    int flag = 0, write_cnt = 0, begin = 17, targetBlkNo = -1, isFinish = 0;//flag:发现的结果数目，write_cnt写入计数器，begin：区块的开始位置 ,isFinish：是否已经搜索结束
    unsigned char *r_blk, *w_blk; /* A pointer to a block */

    if ((r_blk = readBlockFromDisk(begin + 3000, buf)) == NULL) {//没有读取到索引，就创建索引
        perror("Index Block not Found!  Build New Index File\n");
        CreateIndex(buf);
    }
    for (int i = begin; r_blk != NULL && targetBlkNo == -1; i++) {//遍历索引磁盘块，查找索引字段对应的磁盘块
        int first_value = readNumberFromBlk(r_blk, 0, 0);
        int last_value = readNumberFromBlk(r_blk, 6 * 8, 0);
        if (first_value > value) {//如果这个索引区块中的最小值还大于目标值，之后的也会大于目标值，直接break即可{
            targetBlkNo = -1;
            break;
        } else if (last_value < value) {//如果这个索引区块中的最大值还小于目标值，说明需要搜索的索引字段在后面，继续搜索即可
            freeBlockInBuffer(r_blk, buf);//最后释放掉这个读入块
            r_blk = readBlockFromDisk(i + 1 + 3000, buf);
            targetBlkNo = -1;
            continue;
        } else {
            for (int j = 0; j < 7; j++) {//如果目标值小于一个索引磁盘块中的最大值同时小于最小值，则在这个磁盘块中搜索索引字段即可
                if (value == readNumberFromBlk(r_blk, j * 8, 0)) {
                    targetBlkNo = readNumberFromBlk(r_blk, j * 8, 4);
                    break;
                }
            }
        }
        freeBlockInBuffer(r_blk, buf);//最后释放掉这个读入块
    }
    if (targetBlkNo > 0) {//在块内搜索数值
        w_blk = getNewBlockInBuffer(buf);
        memset(w_blk, 0, 64);
        while (isFinish != 1) {
            if ((r_blk = readBlockFromDisk(targetBlkNo, buf)) == NULL) {//没有读取到索引，就创建索引
                perror("Reading Block Failed!\n");
                return;
            }
            for (int j = 0; j < 7; j++) {//遍历区块中所有的值
                int int_C = readNumberFromBlk(r_blk, 8 * j, 0);//将C转为int型
                int int_D = readNumberFromBlk(r_blk, 8 * j, 4);//将C转为int型
                if (int_C == value) {//如果出现过当前字段匹配目标字段
                    printf("%d:(S.C=%d,S.D=%d)\n", flag, int_C, int_D);
                    flag++;
                    if (write_cnt % 7 != 0 || write_cnt == 0) {//块内还有空间或者是第一次写入
                        memcpy(w_blk + (write_cnt % 7) * 8, r_blk + 8 * j, 4);
                        memcpy(w_blk + (write_cnt % 7) * 8 + 4, r_blk + 8 * j + 4, 4);

                    } else {//一个块写满了，将其写回到文件 3100+序号.blk
                        saveBlock(w_blk, buf, 3100 + write_cnt / 7);
                        w_blk = getNewBlockInBuffer(buf);
                        memset(w_blk, 0, 64);
                        memcpy(w_blk, r_blk + 8 * j, 4);
                        memcpy(w_blk + 4, r_blk + 8 * j + 4, 4);
                    }
                    write_cnt++;


                } else {
                    if (flag != 0) {//flag不为0，之后在遇到不为目标字段value的值，说明已经搜索完所有字段为value的元组，完成后break即可
                        isFinish = 1;
                        break;
                    }
                }
            }
            targetBlkNo++;//搜索下一个包含目标字段的磁盘块
        }
        saveBlock(w_blk, buf, 3100 + (write_cnt - 1) / 7 + 1);//结果写入3100+write_cnt/7向上取整
        if (flag == 0) {
            printf("未发现任何结果满足 S.C=%d\n", value);
        } else {
            printf("发现 %d 条结果满足 S.C=%d\n结果已写入文件 %d.blk---%d.blk\n", flag, value, 3101, (write_cnt - 1) / 7 + 1 + 3100);
            return;
        }
    }
}

void SortMergeJoin(Buffer *buf) {
    MergeJoin(buf);
}

void MergeJoin(Buffer *buf) {


    unsigned char *s_blk[4];//用于在缓冲区中暂存s的四路子集合
    unsigned char *r_blk[2];//用于在缓冲区中暂存r的二路子集合

    unsigned char *w_blk = getNewBlockInBuffer(buf);//输出
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//左总游标，范围[0,55]
    int leftR[2];//左总游标，范围[0,55]

    int leftInnerS[4];//左块内部游标，范围[0,6],
    int leftInnerR[2];//左块内部游标，范围[0,6],

    int numberS[4]; //暂存数据S的第leftS[i]的元组的数值
    int numberR[2]; //暂存数据R的第leftR[i]的元组的数值

    const int right = 56;
    for (int i = 0; i < 4; i++) {//初始化关系s
        leftInnerS[i] = 0;
        leftS[i] = 0;
        if ((s_blk[i] = readBlockFromDisk(2000 + i * 8 + 16 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    for (int i = 0; i < 2; i++) {//初始化关系r
        leftR[i] = 0;
        leftInnerR[i] = 0;
        if ((r_blk[i] = readBlockFromDisk(2000 + i * 8 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    while (1) {
        for (int i = 0; i < 4; i++) {
            if (leftS[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerS[i] >= 7) {//已经读取完了一个块
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//释放之前的块
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerR[i] >= 7) {//已经读取完了一个块
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//释放之前的块
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberR[i] = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 0);
        }

        int minSc = 1 << 30, minSd = 1 << 30, minIndexS = -1;
        int minRa = 1 << 30, minRb = 1 << 30, minIndexR = -1;


        for (int i = 0; i < 4; i++) {//选出Sc最小的数和对应组的编号
            if (leftS[i] == right)
                continue;
            if (numberS[i] < minSc) {
                minSc = numberS[i];
                minSd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                minIndexS = i;

            } else if (numberS[i] == minSc) {
                int sd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                if (sd < minSd) {
                    minSc = numberS[i];
                    minSd = sd;
                    minIndexS = i;
                }
            }
        }

        for (int i = 0; i < 2; i++) {//选出Ra最小的数和对应组的编号
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//判断第二位谁更小
                    minRa = numberR[i];
                    minRb = rb;
                    minIndexR = i;
                }
            }
        }


        int sc = minSc;
        int ra = minRa;
        int sd = minSd;
        int rb = minRb;

        if (sc == ra) {//两条记录相同，只选取 一条
            int flg[2] = {0, 0};//标志各条路是否对于当前sc归并完成的标志位
            int curLeftR = leftR[minIndexR];//从R开始数向右遍历
            int curLeftInnerR = leftInnerR[minIndexR];
            unsigned char *curR_blk = r_blk[minIndexR];
            while (sc == ra) {
                if (write_cnt % 3 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                    saveBlock(w_blk, buf, 4100 + write_cnt / 3);
                    w_blk = getNewBlockInBuffer(buf);
                    memset(w_blk, 0, 64);
                }

                printf("%d :（%d）  s.d=%d s.c=%d  r.a=%d r.b=%d （%d）\n", write_cnt, minIndexS + 1, sd, sc, ra, rb,
                       minIndexR + 1);

                char string_ra[3], string_rb[5];
                itoa(ra, string_ra, 10);
                itoa(rb, string_rb, 10);

                memcpy(w_blk + (write_cnt % 3) * 16, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
                memcpy(w_blk + (write_cnt % 3) * 16 + 8, string_ra, 2);
                memcpy(w_blk + (write_cnt % 3) * 16 + 8 + 4, string_rb, 4);
                write_cnt++;

                curLeftR++;
                curLeftInnerR++;
                if (curLeftR == right) {//遍历到最右边
                    flg[minIndexR] = 1;//标记当前路
                } else {
                    if (curLeftInnerR >= 7) {
                        if ((int) curR_blk != (int) r_blk[minIndexR]) {//比较地址
                            freeBlockInBuffer(curR_blk, buf);
                        }
                        if ((curR_blk = readBlockFromDisk(2000 + curLeftR / 7 + minIndexR * 8 + 1, buf)) ==
                            NULL) {//重新读一个块
                            perror("Reading Block Failed!\n");
                            return;
                        }
                        curLeftInnerR = 0;

                    }
                    ra = readNumberFromBlk(curR_blk, curLeftInnerR * 8, 0);
                    rb = readNumberFromBlk(curR_blk, curLeftInnerR * 8, 4);
                }

                if (ra > sc) {
                    flg[minIndexR] = 1;
                }
                if (flg[minIndexR] == 1 && flg[1 - minIndexR] == 1) {//判断双路是否都归并完成，完成则表示当前sc处理完成
                    if ((int) curR_blk != (int) r_blk[minIndexR]) {//比较地址
                        freeBlockInBuffer(curR_blk, buf);
                    }
                    break;
                }
                if (flg[minIndexR] == 1 && flg[1 - minIndexR] != 1) {//换路

                    if (numberR[1 - minIndexR] == sc) {//如果minIndexR所处的一路和sc相等的值已经全部遍历，就切换到另一路进行比较，直到都大于sc
                        if ((int) curR_blk != (int) r_blk[minIndexR]) {//比较地址
                            freeBlockInBuffer(curR_blk, buf);
                        }
//                        printf("switch:%d->%d\n",minIndexR,1-minIndexR);
                        minIndexR = 1 - minIndexR;
                        ra = numberR[minIndexR];
                        curR_blk = r_blk[minIndexR];
                        curLeftR = leftR[minIndexR];//从R开始数向右遍历
                        curLeftInnerR = leftInnerR[minIndexR];
                        rb = readNumberFromBlk(curR_blk, curLeftInnerR * 8, 4);

                    } else {
                        if ((int) curR_blk != (int) r_blk[minIndexR]) {//比较地址
                            freeBlockInBuffer(curR_blk, buf);
                        }
                        break;
                    }
                }


            }

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;

        } else if (sc > ra) {//输出小的，因此输出r
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else {//输出小的，因此输出s sc<ra
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        }


        int _target4[4] = {56, 56, 56, 56};
        int _target2[2] = {56, 56};
        //printf("write_cnt=%d \n",write_cnt);
        if (memcmp(_target2, leftR, 8) == 0 && memcmp(_target4, leftS, 16) == 0) {
            break;
        }
    }
    saveBlock(w_blk, buf, 4100 + (write_cnt - 1) / 3 + 1);
    for (int i = 0; i < 4; i++) {//清空buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//清空buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("连接操作共有 %d 条结果\n结果已写入文件 %d.blk---%d.blk\n", write_cnt, 4101, 4100 + (write_cnt - 1) / 3 + 1);
}


void MergeUnion(Buffer *buf) {


    unsigned char *s_blk[4];
    unsigned char *r_blk[2];

    unsigned char *w_blk = getNewBlockInBuffer(buf);//输出
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//左总游标，范围[0,55]
    int leftR[2];//左总游标，范围[0,55]

    int leftInnerS[4];//左块内部游标，范围[0,6],
    int leftInnerR[2];//左块内部游标，范围[0,6],

    int numberS[4];
    int numberR[2];

    const int right = 56;
    for (int i = 0; i < 4; i++) {
        leftInnerS[i] = 0;
        leftS[i] = 0;
        if ((s_blk[i] = readBlockFromDisk(2000 + i * 8 + 16 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    for (int i = 0; i < 2; i++) {
        leftR[i] = 0;
        leftInnerR[i] = 0;
        if ((r_blk[i] = readBlockFromDisk(2000 + i * 8 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    while (1) {
        for (int i = 0; i < 4; i++) {
            if (leftS[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerS[i] >= 7) {//已经读取完了一个块
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//释放之前的块
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerR[i] >= 7) {//已经读取完了一个块
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//释放之前的块
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberR[i] = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 0);
        }

        int minSc = 1 << 30, minSd = 1 << 30, minIndexS = -1;
        int minRa = 1 << 30, minRb = 1 << 30, minIndexR = -1;

//        printf("numberS:%d %d %d %d\n",numberS[0],numberS[1],numberS[2],numberS[3]);
//        printf("leftS %d %d %d %d\n", leftS[0],leftS[1],leftS[2],leftS[3]);

        for (int i = 0; i < 4; i++) {//选出最小的数和对应组的编号
            if (leftS[i] == right)
                continue;
            if (numberS[i] < minSc) {
                minSc = numberS[i];
                minSd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                minIndexS = i;

            } else if (numberS[i] == minSc) {
                int sd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                if (sd < minSd) {
                    minSc = numberS[i];
                    minSd = sd;
                    minIndexS = i;
                }
            }
        }
//        printf("numberR:%d %d \n",numberR[0],numberR[1]);
//        printf("leftR %d %d \n", leftR[0],leftR[1]);
        for (int i = 0; i < 2; i++) {//选出最小的数和对应组的编号
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//判断第二位谁更小
                    minRa = numberR[i];
                    minRb = rb;
                    minIndexR = i;
                }
            }
        }


        int sc = minSc;
        int ra = minRa;
        int sd = minSd;
        int rb = minRb;
//        printf("%d : (%d) s.d=%d s.c=%d  r.a=%d r.b=%d (%d)\n", write_cnt,minIndexS+1, sd, sc, ra, rb,minIndexR+1);
//        printf("---------------\n");

        if (sc == ra && sd == rb) {//两条记录相同，只选取 一条
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd > rb) {//输出小的，因为sd>rb因此输出r
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, r_blk[minIndexS] + 8 * leftInnerR[minIndexR], 8);

            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd < rb) {//输出小的，因此输出s
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        } else if (sc > ra) {//输出小的，因此输出r
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, r_blk[minIndexR] + 8 * leftInnerR[minIndexR], 8);
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else {//输出小的，因此输出s sc<ra
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        }
        write_cnt++;


        int _target4[4] = {56, 56, 56, 56};
        int _target2[2] = {56, 56};
        //printf("write_cnt=%d \n",write_cnt);
        if (memcmp(_target2, leftR, sizeof(leftR)) == 0 && memcmp(_target4, leftS, sizeof(leftS)) == 0) {
            break;
        }
    }
    saveBlock(w_blk, buf, 5100 + (write_cnt - 1) / 7 + 1);
    for (int i = 0; i < 4; i++) {//清空buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//清空buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("并操作共有 %d 条结果\n结果已写入文件 %d.blk---%d.blk\n", write_cnt, 5101, 5100 + write_cnt / 7);
}


void MergeIntersect(Buffer *buf) {

    unsigned char *s_blk[4];
    unsigned char *r_blk[2];

    unsigned char *w_blk = getNewBlockInBuffer(buf);//输出
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//左总游标，范围[0,55]
    int leftR[2];//左总游标，范围[0,55]

    int leftInnerS[4];//左块内部游标，范围[0,6],
    int leftInnerR[2];//左块内部游标，范围[0,6],

    int numberS[4];
    int numberR[2];

    const int right = 56;
    for (int i = 0; i < 4; i++) {
        leftInnerS[i] = 0;
        leftS[i] = 0;
        if ((s_blk[i] = readBlockFromDisk(2000 + i * 8 + 16 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    for (int i = 0; i < 2; i++) {
        leftR[i] = 0;
        leftInnerR[i] = 0;
        if ((r_blk[i] = readBlockFromDisk(2000 + i * 8 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    while (1) {
        for (int i = 0; i < 4; i++) {
            if (leftS[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerS[i] >= 7) {//已经读取完了一个块
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//释放之前的块
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerR[i] >= 7) {//已经读取完了一个块
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//释放之前的块
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberR[i] = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 0);
        }

        int minSc = 1 << 30, minSd = 1 << 30, minIndexS = -1;
        int minRa = 1 << 30, minRb = 1 << 30, minIndexR = -1;

//        printf("numberS:%d %d %d %d\n",numberS[0],numberS[1],numberS[2],numberS[3]);
//        printf("leftS %d %d %d %d\n", leftS[0],leftS[1],leftS[2],leftS[3]);

        for (int i = 0; i < 4; i++) {//选出最小的数和对应组的编号
            if (leftS[i] == right)
                continue;
            if (numberS[i] < minSc) {
                minSc = numberS[i];
                minSd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                minIndexS = i;

            } else if (numberS[i] == minSc) {
                int sd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                if (sd < minSd) {
                    minSc = numberS[i];
                    minSd = sd;
                    minIndexS = i;
                }
            }
        }
//        printf("numberR:%d %d \n",numberR[0],numberR[1]);
//        printf("leftR %d %d \n", leftR[0],leftR[1]);
        for (int i = 0; i < 2; i++) {//选出最小的数和对应组的编号
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//判断第二位谁更小
                    minRa = numberR[i];
                    minRb = rb;
                    minIndexR = i;
                }
            }
        }


        int sc = minSc;
        int ra = minRa;
        int sd = minSd;
        int rb = minRb;

        if (sc == ra && sd == rb) {//两条记录相同，只选取 一条
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5200 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
            write_cnt++;
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd > rb) {//输出小的，因为sd>rb因此输出r


            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd < rb) {//输出小的，因此输出s

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        } else if (sc > ra) {//输出小的，因此输出r

            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else {//输出小的，因此输出s sc<ra

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        }

        int _target4[4] = {56, 56, 56, 56};
        int _target2[2] = {56, 56};
        //printf("write_cnt=%d \n",write_cnt);
        if (memcmp(_target2, leftR, 8) == 0 && memcmp(_target4, leftS, 16) == 0) {//判断遍历全部的分组
            break;
        }
    }
    saveBlock(w_blk, buf, 5200 + (write_cnt - 1) / 7 + 1);
    for (int i = 0; i < 4; i++) {//清空buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//清空buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("交操作共有 %d 条结果\n结果已写入文件 %d.blk---%d.blk\n", write_cnt, 5201, 5200 + (write_cnt - 1) / 7 + 1);
}

void MergeExcept(Buffer *buf) {

    unsigned char *s_blk[4];
    unsigned char *r_blk[2];

    unsigned char *w_blk = getNewBlockInBuffer(buf);//输出
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//左总游标，范围[0,55]
    int leftR[2];//左总游标，范围[0,55]

    int leftInnerS[4];//左块内部游标，范围[0,6],
    int leftInnerR[2];//左块内部游标，范围[0,6],

    int numberS[4];
    int numberR[2];

    const int right = 56;
    for (int i = 0; i < 4; i++) {
        leftInnerS[i] = 0;
        leftS[i] = 0;
        if ((s_blk[i] = readBlockFromDisk(2000 + i * 8 + 16 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    for (int i = 0; i < 2; i++) {
        leftR[i] = 0;
        leftInnerR[i] = 0;
        if ((r_blk[i] = readBlockFromDisk(2000 + i * 8 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    while (1) {
        for (int i = 0; i < 4; i++) {
            if (leftS[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerS[i] >= 7) {//已经读取完了一个块
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//释放之前的块
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//当前组已经处理完，直接跳过就行
                continue;

            if (leftInnerR[i] >= 7) {//已经读取完了一个块
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//释放之前的块
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//重新读一个块
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberR[i] = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 0);
        }

        int minSc = 1 << 30, minSd = 1 << 30, minIndexS = -1;
        int minRa = 1 << 30, minRb = 1 << 30, minIndexR = -1;

//        printf("numberS:%d %d %d %d\n",numberS[0],numberS[1],numberS[2],numberS[3]);
//        printf("leftS %d %d %d %d\n", leftS[0],leftS[1],leftS[2],leftS[3]);

        for (int i = 0; i < 4; i++) {//选出最小的数和对应组的编号
            if (leftS[i] == right)
                continue;
            if (numberS[i] < minSc) {
                minSc = numberS[i];
                minSd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                minIndexS = i;

            } else if (numberS[i] == minSc) {
                int sd = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 4);
                if (sd < minSd) {
                    minSc = numberS[i];
                    minSd = sd;
                    minIndexS = i;
                }
            }
        }
//        printf("numberR:%d %d \n",numberR[0],numberR[1]);
//        printf("leftR %d %d \n", leftR[0],leftR[1]);
        for (int i = 0; i < 2; i++) {//选出最小的数和对应组的编号
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//判断第二位谁更小
                    minRa = numberR[i];
                    minRb = rb;
                    minIndexR = i;
                }
            }
        }


        int sc = minSc;
        int ra = minRa;
        int sd = minSd;
        int rb = minRb;
//        printf("%d : (%d) s.d=%d s.c=%d  r.a=%d r.b=%d (%d)\n", write_cnt,minIndexS+1, sd, sc, ra, rb,minIndexR+1);
//        printf("---------------\n");

        if (sc == ra && sd == rb) {//两条记录相同，只选取 一条

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd > rb) {//输出小的，因为sd>rb因此输出r

            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd < rb) {//输出小的，因此输出s
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5300 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            write_cnt++;
        } else if (sc > ra) {//输出小的，因此输出r
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;

        } else {//输出小的，因此输出s sc<ra
            if (write_cnt % 7 == 0 && write_cnt != 0) {//一个块写满了，将其写回到文件 4000+序号.blk
                saveBlock(w_blk, buf, 5300 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            write_cnt++;

        }


        int _target4[4] = {56, 56, 56, 56};
        int _target2[2] = {56, 56};
        //printf("write_cnt=%d \n",write_cnt);
        if (memcmp(_target2, leftR, 8) == 0 && memcmp(_target4, leftS, 16) == 0) {//判断遍历全部的分组
            break;
        }
    }
    saveBlock(w_blk, buf, 5300 + (write_cnt - 1) / 7 + 1);
    for (int i = 0; i < 4; i++) {//清空buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//清空buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("差操作（S-R）共有 %d 条结果\n结果已写入文件 %d.blk---%d.blk\n", write_cnt, 5301, 5300 + (write_cnt - 1) / 7 + 1);
    return;
}

void swap(unsigned char *address1, unsigned char *address2, int length) {
    for (int i = 0; i < length; i++) {
        *(address1 + i) = *(address1 + i) ^ *(address2 + i);
        *(address2 + i) = *(address1 + i) ^ *(address2 + i);
        *(address1 + i) = *(address1 + i) ^ *(address2 + i);
    }
}

int readNumberFromBlk(unsigned char *blk, int address, int offset) {
    unsigned char string[4];
    memcpy(string, blk + address + offset, 4);
    return myAtoi(string, 4);//将C转为int型
}

int ShowUserInterface() {
    char input = 0;
    printf("请选择操作:\n");
    printf("1.基于线性搜索的关系选择 : 模拟select S.C, S.D from S where S.C = 50\n");
    printf("2.两阶段多路归并排序算法 :\n");
    printf("3.基于索引的关系选择算法 (请确保已对数据进行排序)模拟select S.C, S.D from S where S.C = 50 :\n");
    printf("4.基于线性搜索的关系选择 (请确保已对数据进行排序)模拟select S.C, S.D, R.A, R.B from S inner join R on S.C = R.A）:\n");
    printf("5.集合操作算法:并 S ∪ R(请确保已对数据进行排序):\n");
    printf("6.集合操作算法:交 S ∩ R(请确保已对数据进行排序):\n");
    printf("7.集合操作算法:差 S - R(请确保已对数据进行排序):\n");

    scanf("%c", &input);
    switch (input) {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        default:
            return 0;
    }
}

void myItoa(int n, char s[]) {
    int i, sign, j, l;
    char c;
    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';

    for (l = 0, j = i - 1; l < j; l++, j--) {
        c = s[l];
        s[l] = s[j];
        s[j] = c;
    }
}

int myAtoi(char *str, int length) {
    // Initialize result
    int res = 0;

    // Iterate through all characters
    // of input string and update result
    // take ASCII character of corresponding digit and
    // subtract the code from '0' to get numerical
    // value and multiply res by 10 to shuffle
    // digits left to update running total
    for (int i = 0; str[i] != '\0' && i < length; ++i)
        res = res * 10 + str[i] - '0';

    // return result.
    return res;
}

void saveBlock(unsigned char *blk, Buffer *buf, int address) {

    char string_next_blk[4];
    myItoa(address + 1, string_next_blk);
    memcpy(blk + 7 * 8, string_next_blk, 4);//写入下一个blk 的地址
    //printf("已写入%d.blk\n",address);
    if (writeBlockToDisk(blk, address, buf) != 0) {
        perror("Writing Block Failed!\n");
    }
}