#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#include <string.h>
/*
 * ʵ��Ҫ��ʹ�������ڴ棨 Buffer ��ʵ�������㷨�����ɶ��峤�ȴ���10�����ͻ��ַ������顣
 * ��ϵR������������A��B�� A��ֵ��Ϊ[20, 60]��B��ֵ��Ϊ[1000, 2000]��
 * ��ϵS������������C��D��C��ֵ��Ϊ[40, 80]��D��ֵ��Ϊ[1000, 3000]��
 * ����ֵ��Ϊint�ͣ�4���ֽڣ���R��S��ÿ��Ԫ��Ĵ�С��Ϊ8���ֽڡ�
 *  1. ��Ŀ�е�����ѡ���������ӣ� ���ǻ��ڹ�ϵ�ĵ�һ�����ԣ���R.A �� S.C��
    2. ʵ�����ݾ����޸ģ�������ϵһ����15����ͬ��Ԫ�顣Ϊ�����ҵ��ԣ���������5����ͬ��Ԫ�飺(40,1243),(41,1829),(42,1422),(58,1256),(58,1747)��
    3. ʵ����ֻ����ʹ���ڴ滺������520�ֽڣ����������з����Ľ���ҪBuffer�н��У� ���ɶ���size>10�����顣
    4. ����ֻ����Buffer�ڣ�Ҳ���ɶ�����size<10����������������
    5. ���ϵĲ����������Ƕ����������㷨�� ���Ͳ�������������ý����Ľ����
    6. ������ɵĹ�ϵS�д�����ͬԪ�飬�����滻dataĿ¼�µ�25.blkΪ���ϴ����ļ���

 */
/**
 * �Զ����ַ���ת���֣���֤���ᷢ������Խ������ĸ����ã������ַ������Ľ�����ʾ��\0����
 * @param str ��Ҫת�����ַ���
 * @param length  ת���ĳ���
 * @return ת�����
 */
int myAtoi(char *str, int length);

/**
 * ����ת�ַ���
 * @param n ��Ҫת��������
 * @param s  ���ø����ֵ��ַ���
 */
void myItoa(int, char *s);

/**
 * չʾ�û�����
 * @return �û�ѡ���ָ��
 */
int ShowUserInterface();

/**
 * ʹ��������ԭ���address1��address2��Ӧ��ַ�г���Ϊlength�����ݽ��н���
 * @param address1  ��ַ1
 * @param address2  ��ַ2
 * @param length  �����ĳ���
 */
void swap(unsigned char *address1, unsigned char *address2, int length);

/**
 * �����ݿ�blkд�����
 * @param blk ���ݿ�
 * @param buf ������
 * @param address д��ĵ�ַ
 */
void saveBlock(unsigned char *blk, Buffer *buf, int address);

/**
 * �ӿ�blk �ж�ȡλ��address+offsetλ�õ��ַ���ת������������
 * @param blk Ŀ���
 * @param address �ַ���ʼ�ĵ�ַ ͨ����8�ı���
 * @param offset  ƫ���� ͨ����0��4
 * @return address+offsetλ�õ��ַ���Ӧ����������
 */
int readNumberFromBlk(unsigned char *blk, int address, int offset);

/**
 * ʵ�ֻ������������Ĺ�ϵѡ���㷨������ExtMem����⣬ʹ��C����ʵ�����������㷨��ѡ��S.C=50��Ԫ�飬��¼IO��д����������ѡ��������ڴ����ϡ�
 * ��ģ��ʵ�� select S.C,S.D from S where S.C = 50��

 * @param buf ������
 * @param value ��������ֵ
 */
void LinearSearch(Buffer *buf, int value);

/**
 * ���Ȼ����Ӽ����Ӽ�����
 * DivideSort()���������ݿ�Ŀ�����Ӽ����֣���ʹ��ð���������Ӽ��ж�����Ԫ��������򣬲���������浽���ݿ���
 * @param buf ������
 * @param begin �Ӽ��ڳ�ʼ�����ݵ���ʼλ��
 * @param end �Ӽ��ڳ�ʼ�����ݵ�����ֹλ��
 */
void DivideSort(Buffer *buf, int begin, int end);

/**
  (��) ���Ӽ���鲢����
   ���������ζ�ȡһ��Ԫ���γɴ��Ƚϼ��ϣ������Ƚϼ����е���СԪ��д�������
   ԭ����Ƿֱ��ȡÿһ���Ӽ�����С����ֵ����Ϊ�Ӽ��Ѿ���������ˣ���ʵ���Ƕ�ȡ�Ӽ����ǰ����ֵ����Ȼ���ڲ�ͬ�Ӽ�֮����бȽϣ�ѡ�������С�ģ�ֱ�����е��Ӽ��е���ֵ������꼴��
 * @param buf ������
 * @param begin �Ӽ��ڳ�ʼ�����ݵ���ʼλ��
 * @param end �Ӽ��ڳ�ʼ�����ݵ�����ֹλ��
 * @param outputBlkNum ��������Ŀ
 */
void MergeSort(Buffer *buf, int begin, int end, int outputBlkNum);

/**
 * ���׶ζ�·�鲢�����㷨
 * @param buf ������
 * @param source  ѡ��Ĺ�ϵ
 */
void TPMMS(Buffer *buf, char source);

/**
 * ���ɹ�ϵS�������ļ�
 * ���������Ĺ��������ڵڶ��ʵ����������������̾��Ǵ�С����������������Գ��ֵ�ÿһ����ͬ�������ֶμ�¼���һ�γ��ֵĴ��̿飬��������Ϣ�������ֶ���Ϣһͬ��¼�������ļ���
 * @param buf ������
 */
void CreateIndex(Buffer *buf);

/**
 *  ʵ�ֻ��������Ĺ�ϵѡ���㷨�����ã�2���е�������Ϊ��ϵS���������ļ�������������
    ��ѡ��S.C=50��Ԫ�飬����ѡ��������ڴ����ϡ���¼IO��д�������루1���еĽ����
�ȡ���ģ��ʵ�� select S.C, S.D from S where S.C = 50 ��
   �ڽ��������󣬼��ɻ���������������,���ȶ�ȡ�����ļ������������ֶλ�ȡ��������������λ�ã�targetBlkNo����
   Ȼ�����ڸ������в���ȫ��������������ֵ����Ϊ�������Ѿ��ź���ģ��������������ֶζ�Ӧ�����ݵ���Ŀ�ϴ�ͻ�ֲ��ڶ�������У�������һ�������������һ�������м���������ֱ��������ͬ�������ֶΣ�������ֹ�������������־û������̿鼴��
   ���û�з��������ֶΣ���˵������û�в������
 * @param buf ������
 * @param value���ص���ֵ
 */
void IndexSearch(Buffer *buf, int value);

/**
 *   ʵ�ֻ�����������Ӳ����㷨��Sort-Merge-Join�����Թ�ϵS��R����S.C����R.A ����ͳ��
    ���Ӵ����������ӽ������ڴ����ϡ� ��ģ��ʵ�� select S.C, S.D, R.A, R.B from S inner
    join R on S.C = R.A��
 */
void SortMergeJoin(Buffer *buf);

/**
 * �Էֿ������Ľ�����ڲ��������й鲢
    �������ִ�й鲢�׶Σ��������̾��Ƕ���S��R�����ϵ��ÿһ����һ��ѡ����С����ֵ��
    Ȼ�����ߣ�sc��ra�����бȽϣ��������ͬ�Ļ��򵯳������еĽ�Сֵ��Ȼ�������С�ߣ����������ͬ�����ж�rb��sd���������С�ڹ�ϵ�Ļ��������С��ֵ��Ӧ��Ԫ��
   ���������ͬ�Ļ����ݲ�������ֻ���һ��Ԫ�鼴��
 * @param buf ������
 */
void MergeUnion(Buffer *buf);

/**
 * �Էֿ������Ľ�����ڽ��������й鲢
 * �������ִ�й鲢�׶Σ��������̾��Ƕ���S��R�����ϵ��ÿһ����һ��ѡ����С����ֵ��
    Ȼ�����ߣ�sc��ra�����бȽϣ��������ͬ�Ļ��򵯳������еĽ�Сֵ��Ȼ��������С�ߣ����������ͬ�����ж�rb��sd�����������ͬ�Ļ�������������Ľ��������������С��Ԫ������Ƚ�
 * @param buf ������
 */
void MergeIntersect(Buffer *buf);

/**
 * �Էֿ������Ľ�����ڲ�������й鲢
 * �������ִ�й鲢�׶Σ��������̾��Ƕ���S��R�����ϵ��ÿһ����һ��ѡ����С����ֵ��
    Ȼ�����ߣ�sc��ra�����бȽϣ��������ͬ�Ļ��򵯳������еĽ�Сֵ�������С����sc�Ļ������sc�������������������ra�����������ͬ�����ж�rb��sd�����������ͬ�Ļ���������sc��ֱ��sc��ra���鲢���
 * @param buf ������
 */
void MergeExcept(Buffer *buf);

/**
 * �Էֿ������Ľ���������Ӳ������й鲢
 * �������ִ�й鲢�׶Σ��������̾��Ƕ���S��R�����ϵ��ÿһ����һ��ѡ����С����ֵ��Ȼ�����ߣ�sc��ra�����бȽϣ��������ͬ�Ļ��򵯳������еĽ�Сֵ��
   ֱ��������ͬ��Ȼ���ٽ������ӣ�������scΪ��׼��������·��������sc��ȵ�ra����������Ԫ��һ��д���������
 * @param buf ������
 */
void MergeJoin(Buffer *buf);


 /**
  * ʵ�ֻ��������ɢ�е�����ɨ���㷨��ʵ�ֲ���S R�� ������S R�� ���S - R������
һ�ּ��ϲ����㷨�����������ڴ����ϣ���ͳ�Ʋ���������������Ԫ�������
 �������ǽ�������ϵ��Ԫ��ϲ���һ����ϵ���ںϲ�ʱȥ���ظ���Ԫ��
  * @param buf ������
  */
void Union(Buffer *buf) {
    MergeUnion(buf);
}
/**
 * ʵ�ֻ��������ɢ�е�����ɨ���㷨��ʵ�ֲ���S R�� ������S R�� ���S - R������
һ�ּ��ϲ����㷨�����������ڴ����ϣ���ͳ�Ʋ���������������Ԫ�������
�������ǽ�������ϵ��Ԫ��ϲ���һ����ϵ���ںϲ�ʱȥ���ظ���Ԫ��
 * @param buf ������
 */
void Intersect(Buffer *buf) {
    MergeIntersect(buf);
}
/**
 * ʵ�ֻ��������ɢ�е�����ɨ���㷨��ʵ�ֲ���S R�� ������S R�� ���S - R������
һ�ּ��ϲ����㷨�����������ڴ����ϣ���ͳ�Ʋ���������������Ԫ�������
�������ǽ�������ϵ��Ԫ��ϲ���һ����ϵ���ںϲ�ʱȥ���ظ���Ԫ��
 * @param buf ������
 */
void Except(Buffer *buf) {
    MergeExcept(buf);
}
/**
 * ��������ûʲô��˵��
 * @param argc ������Ŀ
 * @param argv  ��������
 * @return  �������еĻ�����0������᷵���쳣ֵ
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
        printf("��������ȷ��ָ��\n");
    }
    printf("IO's is %lu\n", buf.numIO); /* Check the number of IO's */
    return 0;
}

void LinearSearch(Buffer *buf, int value) {
    int i, j;
    int flag = 0;
    unsigned char *r_blk; //��ȡ������
    unsigned char *w_blk = getNewBlockInBuffer(buf); //���������
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int next_blk = 1;
    for (i = 17; i <= 48; i++) {
        printf("�������ݿ飺%d.blk\n", i);
        if ((r_blk = readBlockFromDisk(i, buf)) == NULL) {//��ȡ���̿�
            perror("Reading Block Failed!\n");
            return;
        }
        for (j = 0; j < 7; j++) {//һ�������װ7������
            int int_C = readNumberFromBlk(r_blk, 8 * j, 0);//�����̿��е�CתΪint��
            int int_D = readNumberFromBlk(r_blk, 8 * j, 4);//�����̿��е�DתΪint��
            if (int_C == value) {//�ж���ֵ�Ƿ�ƥ��
                printf("%d:(S.C=%d,S.D=%d)\n", flag, int_C, int_D);
                flag++;
                if (next_blk == 1) {//������������w_blk��û��д��
                    next_blk++; //��һ����102
                }
                if (write_cnt == 7) {//���һ�����Ѿ�д���ˣ��ͽ��������saveBlock()���������ύ
                    saveBlock(w_blk, buf, 1000 + next_blk - 1);//����Ϊ1000+x��xΪ��ǰд������ţ�
                    w_blk = getNewBlockInBuffer(buf);//�ύ���ȡһ���µĿ�
                    memset(w_blk, 0, 64);
                    write_cnt = 0;//����write_cntΪ0�����ڼ�������
                    next_blk++;
                }
                memcpy(w_blk + write_cnt * 8, r_blk + 8 * j, 4);//��r_blk�е�s.cд������� write_cnt * 8�ĵ�ַ
                memcpy(w_blk + write_cnt * 8 + 4, r_blk + 8 * j + 4, 4);//��r_blk�е�s.dд������� write_cnt * 8 +4�ĵ�ַ
                write_cnt++;
            }
        }
        freeBlockInBuffer((unsigned char *) r_blk, buf);//������һ�����̿�󣬽���ռ�õ�buffer�ͷŵ�

    }
    if (flag == 0) {
        printf("δ�����κν������ S.C=%d\n", value);
    } else {
        saveBlock(w_blk, buf, 1000 + next_blk - 1);//�ύѭ������ǰ��δ�ύ�����һ����
        printf("���� %d ��������� S.C=%d\n�����д���ļ� %d.blk---%d.blk\n", flag, value, 1001, next_blk + 1000 - 1);
        return;
    }

}

void DivideSort(Buffer *buf, int begin, int end) {
    unsigned char *blk_array[buf->numAllBlk];//ʹ�����еĻ����������ڴ����м���
    //���������ݲ���һ��װ���ڴ棬�轫���ݷ���װ���������

    for (int outer = begin; outer <= end; outer += 8) {//�������еĿ�
        int inner = 0;
        for (inner = 0; inner < 8; inner++) {

            if ((blk_array[inner] = readBlockFromDisk(outer + inner, buf)) == NULL) {
                perror("Reading Block Failed!\n");
                return;
            }
        }
        printf("�Ѷ�ȡ:%d.blk->.%d.blk��������\n", outer, outer + inner - 1);
        //printf("�ڻ�������ִ��������\n");

        //һ�����������װ8�����ݣ�ÿ���а���7��Ԫ�飬һ��Ԫ������������ݣ����յ�һ������
        int i, j, k, l;
        //ʵ��һ��ð������  ��ѭ���� 0�� len-1 ��ѭ���� 0�� len-1-i
        for (i = 0; i < inner; i++) {//���ѭ��   ��ȡ������ÿ����
            for (j = 0; i != inner - 1 ? j < 7 : j < 6; j++) { //��ȡÿ���а���7��Ԫ��
                int cnt = 56 - 1 - (i * 7 + j);
                for (k = 0; k <= cnt / 7; k++) {
                    for (l = 0; k == cnt / 7 ? l < cnt % 7 : l < 7; l++) {
                        //printf("%d*7+%d=%d\n",k,l,k*7+l);
                        int a_one = readNumberFromBlk(blk_array[k], 8 * l, 0);
                        int a_two = readNumberFromBlk(blk_array[k], 8 * l, 4);
                        int b_one = -1;
                        int b_two = -1;

                        //����������λ�õ�Ԫ�������鴢��,�ʹ���һ��Ŀ�ͷ��ȡ
                        if (l == 6 && k != inner - 1) {
                            b_one = readNumberFromBlk(blk_array[k + 1], 0, 0);
                            b_two = readNumberFromBlk(blk_array[k + 1], 0, 4);

                        } else {//����ͬһ����
                            b_one = readNumberFromBlk(blk_array[k], 8 * (l + 1), 0);
                            b_two = readNumberFromBlk(blk_array[k], 8 * (l + 1), 4);

                        }

                        if (a_one > b_one || (a_one == b_one && a_two > b_two)) { //����˳��
                            unsigned char *address1 = blk_array[k] + 8 * l;
                            unsigned char *address2;
                            if (l == 6 && k != inner - 1) {//�ֱ���������ʱ
                                address2 = blk_array[k + 1];
                            } else {//����ͬһ����
                                address2 = blk_array[k] + 8 * (l + 1);
                            }
                            swap(address1, address2, 8);//ʹ��������swap()����
                        }
                    }
                }
            }
        }
        //ð�������д��2000+ԭ�������λ��
        for (inner = 0; inner < 8; inner++) {
            saveBlock(blk_array[inner], buf, outer + inner + 2000);
        }
    }
}

void MergeSort(Buffer *buf, int begin, int end, int outputBlkNum) {

    unsigned char *in_blk_array[outputBlkNum];//��ȡ�����������
    unsigned char *out_blk = getNewBlockInBuffer(buf);//����飬��СΪ1
    memset(out_blk, 0, 64);
    int write_cnt = 0; //����ļ�����
    int number[outputBlkNum];//����ȽϵĶ���
    int left[outputBlkNum];//�����α꣬��Χ[0,55]
    int leftInner[outputBlkNum];//����ڲ��α꣬��Χ[0,6],
    int right[outputBlkNum];//���α�,����7*8=56
    for (int i = 0; i < outputBlkNum; i++) {//���ȶԻ��ֺ��ÿһ���Ӽ����г�ʼ��
        left[i] = 0; //��ʼ�������α�
        leftInner[i] = 0;
        right[i] = 8 * 7;//ÿ������8����
        if ((in_blk_array[i] = readBlockFromDisk(2000 + i * 8 + begin, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    int flag = 0;//flag�����жϹ鲢����ֹ����
    while (flag == 0) {
        for (int i = 0; i < outputBlkNum; i++) {
            if (left[i] == right[i])//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInner[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInner[i] = 0;
                freeBlockInBuffer(in_blk_array[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((in_blk_array[i] = readBlockFromDisk(2000 + left[i] / 7 + i * 8 + begin, buf)) == NULL) {//���¶�һ����
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            number[i] = readNumberFromBlk(in_blk_array[i], leftInner[i] * 8, 0);
        }
        int min = 1 << 30, minIndex = -1;
        for (int i = 0; i < outputBlkNum; i++) {//ѡ����С�����Ͷ�Ӧ��ı��
            if (left[i] == right[i])
                continue;

            min = number[i] < min ? number[i] : min;
            minIndex = number[i] == min ? i : minIndex;
        }
        //printf("minIndex=%d num=%d\n",minIndex,min);
        //������Сֵ
        if (write_cnt % 7 != 0 || write_cnt == 0) {//һ�������д���ˣ��ͽ����ύ�������µĿ�
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
        for (int i = 0; i < outputBlkNum; i++) {//�ж��Ƿ�ȫ����ȡ�꣬��������е����α�����α��Ƿ���ȣ���ȼ��Ƕ�ȡ���
            if (left[i] != right[i]) {
                //printf("i=%d,left[i]=%d\n",i,left[i]);
                flag = 0;
                break;
            } else {
                flag = 1;
            }
        }
    }
    saveBlock(out_blk, buf, 2100 + (write_cnt - 1) / 7 + 1 + begin - 1);//���������鱣��2100+write_cnt/7(����ȡ��)+��ʼλ��
    for (int i = 0; i < outputBlkNum; i++) {//�ͷ�ȫ���Ӽ����ڶ���ĵ�buffer
        freeBlockInBuffer(in_blk_array[i], buf);
    }
}

void TPMMS(Buffer *buf, char source) {
    int begin, end, outputBlkNum;//�������ݿ�������յ�,ѭ��������

    switch (source) { //�������ݿ�ѡ����и�ֵ
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
            perror("ѡ������ݿ����");
            return;
    }
    DivideSort(buf, begin, end);
    MergeSort(buf, begin, end, outputBlkNum);
    printf("��ϵ%c�����������,�������� %d.blk->%d.blk\n", source, 2100 + begin, 2100 + end);

}

void CreateIndex(Buffer *buf) {
    unsigned char *r_blk; //ʹ��һ���������Ŀ���ж���
    unsigned char *w_blk = getNewBlockInBuffer(buf); /* A pointer to a block */
    memset(w_blk, 0, 64);
    int write_cnt = 0;//д�����ļ�����
    int last_value = 1 << 31, cur_value = 0;//last_valueά����ǰ���ʹ�����Сֵ��cur_value�򱣴浱ǰ���ڷ��ʵ�Ԫ�����ֵ
    int begin = 17, end = 48;//��Ŀ�ʼ�ͽ���
    for (int i = begin; i <= end; i++) {
        if ((r_blk = readBlockFromDisk(i + 2100, buf)) == NULL) {//��ȡ������
            perror("Reading Sorted Block Failed!\n");
            return;
        }
        for (int j = 0; j < 7; j++) {
            cur_value = readNumberFromBlk(r_blk, 8 * j, 0);//��¼��ǰ�����ֶε�ֵ
            //printf("cur=%d,last=%d\n",cur_value,last_value);
            if (cur_value > last_value) {//��������µ������ֶΣ���֮ǰ�κ������ֶζ�Ҫ��
                char string_blkNo[4];//װ�ص�ǰ�����Ϣ
                myItoa(i + 2100, string_blkNo);//�����ת��Ϊ�ַ�����u����
                if (write_cnt % 7 != 0 || write_cnt == 0) {
                    memcpy(w_blk + (write_cnt % 7) * 8, r_blk + 8 * j, 4);//���������ֶ�д��������� (write_cnt % 7) * 8��λ��
                    memcpy(w_blk + (write_cnt % 7) * 8 + 4, string_blkNo, 4);//���������ֶζ�Ӧ��һ�����ԵĿ���д��������� (write_cnt % 7) * 8��λ��

                } else {//һ����д���ˣ�����д�ص��ļ� 3000+���.blk
                    saveBlock(w_blk, buf, 3000 + write_cnt / 7 + begin - 1);
                    w_blk = getNewBlockInBuffer(buf);
                    memset(w_blk, 0, 64);
                    memcpy(w_blk, r_blk + 8 * j, 4);
                    memcpy(w_blk + 4, string_blkNo, 4);

                }
                write_cnt++;
                last_value = cur_value;//���¼�¼
            }
        }
        freeBlockInBuffer(r_blk, buf);//�ͷŶ�ȡ�Ļ���
    }
    if (write_cnt % 7 != 0) //��������Ա��д�뻺����������ֵûд
        saveBlock(w_blk, buf, 3000 + write_cnt / 7 + begin);

}

void IndexSearch(Buffer *buf, int value) {
    int flag = 0, write_cnt = 0, begin = 17, targetBlkNo = -1, isFinish = 0;//flag:���ֵĽ����Ŀ��write_cntд���������begin������Ŀ�ʼλ�� ,isFinish���Ƿ��Ѿ���������
    unsigned char *r_blk, *w_blk; /* A pointer to a block */

    if ((r_blk = readBlockFromDisk(begin + 3000, buf)) == NULL) {//û�ж�ȡ���������ʹ�������
        perror("Index Block not Found!  Build New Index File\n");
        CreateIndex(buf);
    }
    for (int i = begin; r_blk != NULL && targetBlkNo == -1; i++) {//�����������̿飬���������ֶζ�Ӧ�Ĵ��̿�
        int first_value = readNumberFromBlk(r_blk, 0, 0);
        int last_value = readNumberFromBlk(r_blk, 6 * 8, 0);
        if (first_value > value) {//���������������е���Сֵ������Ŀ��ֵ��֮���Ҳ�����Ŀ��ֵ��ֱ��break����{
            targetBlkNo = -1;
            break;
        } else if (last_value < value) {//���������������е����ֵ��С��Ŀ��ֵ��˵����Ҫ�����������ֶ��ں��棬������������
            freeBlockInBuffer(r_blk, buf);//����ͷŵ���������
            r_blk = readBlockFromDisk(i + 1 + 3000, buf);
            targetBlkNo = -1;
            continue;
        } else {
            for (int j = 0; j < 7; j++) {//���Ŀ��ֵС��һ���������̿��е����ֵͬʱС����Сֵ������������̿������������ֶμ���
                if (value == readNumberFromBlk(r_blk, j * 8, 0)) {
                    targetBlkNo = readNumberFromBlk(r_blk, j * 8, 4);
                    break;
                }
            }
        }
        freeBlockInBuffer(r_blk, buf);//����ͷŵ���������
    }
    if (targetBlkNo > 0) {//�ڿ���������ֵ
        w_blk = getNewBlockInBuffer(buf);
        memset(w_blk, 0, 64);
        while (isFinish != 1) {
            if ((r_blk = readBlockFromDisk(targetBlkNo, buf)) == NULL) {//û�ж�ȡ���������ʹ�������
                perror("Reading Block Failed!\n");
                return;
            }
            for (int j = 0; j < 7; j++) {//�������������е�ֵ
                int int_C = readNumberFromBlk(r_blk, 8 * j, 0);//��CתΪint��
                int int_D = readNumberFromBlk(r_blk, 8 * j, 4);//��CתΪint��
                if (int_C == value) {//������ֹ���ǰ�ֶ�ƥ��Ŀ���ֶ�
                    printf("%d:(S.C=%d,S.D=%d)\n", flag, int_C, int_D);
                    flag++;
                    if (write_cnt % 7 != 0 || write_cnt == 0) {//���ڻ��пռ�����ǵ�һ��д��
                        memcpy(w_blk + (write_cnt % 7) * 8, r_blk + 8 * j, 4);
                        memcpy(w_blk + (write_cnt % 7) * 8 + 4, r_blk + 8 * j + 4, 4);

                    } else {//һ����д���ˣ�����д�ص��ļ� 3100+���.blk
                        saveBlock(w_blk, buf, 3100 + write_cnt / 7);
                        w_blk = getNewBlockInBuffer(buf);
                        memset(w_blk, 0, 64);
                        memcpy(w_blk, r_blk + 8 * j, 4);
                        memcpy(w_blk + 4, r_blk + 8 * j + 4, 4);
                    }
                    write_cnt++;


                } else {
                    if (flag != 0) {//flag��Ϊ0��֮����������ΪĿ���ֶ�value��ֵ��˵���Ѿ������������ֶ�Ϊvalue��Ԫ�飬��ɺ�break����
                        isFinish = 1;
                        break;
                    }
                }
            }
            targetBlkNo++;//������һ������Ŀ���ֶεĴ��̿�
        }
        saveBlock(w_blk, buf, 3100 + (write_cnt - 1) / 7 + 1);//���д��3100+write_cnt/7����ȡ��
        if (flag == 0) {
            printf("δ�����κν������ S.C=%d\n", value);
        } else {
            printf("���� %d ��������� S.C=%d\n�����д���ļ� %d.blk---%d.blk\n", flag, value, 3101, (write_cnt - 1) / 7 + 1 + 3100);
            return;
        }
    }
}

void SortMergeJoin(Buffer *buf) {
    MergeJoin(buf);
}

void MergeJoin(Buffer *buf) {


    unsigned char *s_blk[4];//�����ڻ��������ݴ�s����·�Ӽ���
    unsigned char *r_blk[2];//�����ڻ��������ݴ�r�Ķ�·�Ӽ���

    unsigned char *w_blk = getNewBlockInBuffer(buf);//���
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//�����α꣬��Χ[0,55]
    int leftR[2];//�����α꣬��Χ[0,55]

    int leftInnerS[4];//����ڲ��α꣬��Χ[0,6],
    int leftInnerR[2];//����ڲ��α꣬��Χ[0,6],

    int numberS[4]; //�ݴ�����S�ĵ�leftS[i]��Ԫ�����ֵ
    int numberR[2]; //�ݴ�����R�ĵ�leftR[i]��Ԫ�����ֵ

    const int right = 56;
    for (int i = 0; i < 4; i++) {//��ʼ����ϵs
        leftInnerS[i] = 0;
        leftS[i] = 0;
        if ((s_blk[i] = readBlockFromDisk(2000 + i * 8 + 16 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    for (int i = 0; i < 2; i++) {//��ʼ����ϵr
        leftR[i] = 0;
        leftInnerR[i] = 0;
        if ((r_blk[i] = readBlockFromDisk(2000 + i * 8 + 1, buf)) == NULL) {
            perror("Reading Block Failed!\n");
            return;
        }
    }
    while (1) {
        for (int i = 0; i < 4; i++) {
            if (leftS[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerS[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//���¶�һ����
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerR[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//���¶�һ����
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberR[i] = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 0);
        }

        int minSc = 1 << 30, minSd = 1 << 30, minIndexS = -1;
        int minRa = 1 << 30, minRb = 1 << 30, minIndexR = -1;


        for (int i = 0; i < 4; i++) {//ѡ��Sc��С�����Ͷ�Ӧ��ı��
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

        for (int i = 0; i < 2; i++) {//ѡ��Ra��С�����Ͷ�Ӧ��ı��
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//�жϵڶ�λ˭��С
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

        if (sc == ra) {//������¼��ͬ��ֻѡȡ һ��
            int flg[2] = {0, 0};//��־����·�Ƿ���ڵ�ǰsc�鲢��ɵı�־λ
            int curLeftR = leftR[minIndexR];//��R��ʼ�����ұ���
            int curLeftInnerR = leftInnerR[minIndexR];
            unsigned char *curR_blk = r_blk[minIndexR];
            while (sc == ra) {
                if (write_cnt % 3 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
                    saveBlock(w_blk, buf, 4100 + write_cnt / 3);
                    w_blk = getNewBlockInBuffer(buf);
                    memset(w_blk, 0, 64);
                }

                printf("%d :��%d��  s.d=%d s.c=%d  r.a=%d r.b=%d ��%d��\n", write_cnt, minIndexS + 1, sd, sc, ra, rb,
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
                if (curLeftR == right) {//���������ұ�
                    flg[minIndexR] = 1;//��ǵ�ǰ·
                } else {
                    if (curLeftInnerR >= 7) {
                        if ((int) curR_blk != (int) r_blk[minIndexR]) {//�Ƚϵ�ַ
                            freeBlockInBuffer(curR_blk, buf);
                        }
                        if ((curR_blk = readBlockFromDisk(2000 + curLeftR / 7 + minIndexR * 8 + 1, buf)) ==
                            NULL) {//���¶�һ����
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
                if (flg[minIndexR] == 1 && flg[1 - minIndexR] == 1) {//�ж�˫·�Ƿ񶼹鲢��ɣ�������ʾ��ǰsc�������
                    if ((int) curR_blk != (int) r_blk[minIndexR]) {//�Ƚϵ�ַ
                        freeBlockInBuffer(curR_blk, buf);
                    }
                    break;
                }
                if (flg[minIndexR] == 1 && flg[1 - minIndexR] != 1) {//��·

                    if (numberR[1 - minIndexR] == sc) {//���minIndexR������һ·��sc��ȵ�ֵ�Ѿ�ȫ�����������л�����һ·���бȽϣ�ֱ��������sc
                        if ((int) curR_blk != (int) r_blk[minIndexR]) {//�Ƚϵ�ַ
                            freeBlockInBuffer(curR_blk, buf);
                        }
//                        printf("switch:%d->%d\n",minIndexR,1-minIndexR);
                        minIndexR = 1 - minIndexR;
                        ra = numberR[minIndexR];
                        curR_blk = r_blk[minIndexR];
                        curLeftR = leftR[minIndexR];//��R��ʼ�����ұ���
                        curLeftInnerR = leftInnerR[minIndexR];
                        rb = readNumberFromBlk(curR_blk, curLeftInnerR * 8, 4);

                    } else {
                        if ((int) curR_blk != (int) r_blk[minIndexR]) {//�Ƚϵ�ַ
                            freeBlockInBuffer(curR_blk, buf);
                        }
                        break;
                    }
                }


            }

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;

        } else if (sc > ra) {//���С�ģ�������r
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else {//���С�ģ�������s sc<ra
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
    for (int i = 0; i < 4; i++) {//���buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//���buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("���Ӳ������� %d �����\n�����д���ļ� %d.blk---%d.blk\n", write_cnt, 4101, 4100 + (write_cnt - 1) / 3 + 1);
}


void MergeUnion(Buffer *buf) {


    unsigned char *s_blk[4];
    unsigned char *r_blk[2];

    unsigned char *w_blk = getNewBlockInBuffer(buf);//���
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//�����α꣬��Χ[0,55]
    int leftR[2];//�����α꣬��Χ[0,55]

    int leftInnerS[4];//����ڲ��α꣬��Χ[0,6],
    int leftInnerR[2];//����ڲ��α꣬��Χ[0,6],

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
            if (leftS[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerS[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//���¶�һ����
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerR[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//���¶�һ����
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

        for (int i = 0; i < 4; i++) {//ѡ����С�����Ͷ�Ӧ��ı��
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
        for (int i = 0; i < 2; i++) {//ѡ����С�����Ͷ�Ӧ��ı��
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//�жϵڶ�λ˭��С
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

        if (sc == ra && sd == rb) {//������¼��ͬ��ֻѡȡ һ��
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd > rb) {//���С�ģ���Ϊsd>rb������r
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, r_blk[minIndexS] + 8 * leftInnerR[minIndexR], 8);

            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd < rb) {//���С�ģ�������s
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        } else if (sc > ra) {//���С�ģ�������r
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
                saveBlock(w_blk, buf, 5100 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, r_blk[minIndexR] + 8 * leftInnerR[minIndexR], 8);
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else {//���С�ģ�������s sc<ra
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
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
    for (int i = 0; i < 4; i++) {//���buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//���buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("���������� %d �����\n�����д���ļ� %d.blk---%d.blk\n", write_cnt, 5101, 5100 + write_cnt / 7);
}


void MergeIntersect(Buffer *buf) {

    unsigned char *s_blk[4];
    unsigned char *r_blk[2];

    unsigned char *w_blk = getNewBlockInBuffer(buf);//���
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//�����α꣬��Χ[0,55]
    int leftR[2];//�����α꣬��Χ[0,55]

    int leftInnerS[4];//����ڲ��α꣬��Χ[0,6],
    int leftInnerR[2];//����ڲ��α꣬��Χ[0,6],

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
            if (leftS[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerS[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//���¶�һ����
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerR[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//���¶�һ����
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

        for (int i = 0; i < 4; i++) {//ѡ����С�����Ͷ�Ӧ��ı��
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
        for (int i = 0; i < 2; i++) {//ѡ����С�����Ͷ�Ӧ��ı��
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//�жϵڶ�λ˭��С
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

        if (sc == ra && sd == rb) {//������¼��ͬ��ֻѡȡ һ��
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
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
        } else if (sc == ra && sd > rb) {//���С�ģ���Ϊsd>rb������r


            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd < rb) {//���С�ģ�������s

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        } else if (sc > ra) {//���С�ģ�������r

            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else {//���С�ģ�������s sc<ra

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
        }

        int _target4[4] = {56, 56, 56, 56};
        int _target2[2] = {56, 56};
        //printf("write_cnt=%d \n",write_cnt);
        if (memcmp(_target2, leftR, 8) == 0 && memcmp(_target4, leftS, 16) == 0) {//�жϱ���ȫ���ķ���
            break;
        }
    }
    saveBlock(w_blk, buf, 5200 + (write_cnt - 1) / 7 + 1);
    for (int i = 0; i < 4; i++) {//���buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//���buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("���������� %d �����\n�����д���ļ� %d.blk---%d.blk\n", write_cnt, 5201, 5200 + (write_cnt - 1) / 7 + 1);
}

void MergeExcept(Buffer *buf) {

    unsigned char *s_blk[4];
    unsigned char *r_blk[2];

    unsigned char *w_blk = getNewBlockInBuffer(buf);//���
    memset(w_blk, 0, 64);
    int write_cnt = 0;
    int leftS[4];//�����α꣬��Χ[0,55]
    int leftR[2];//�����α꣬��Χ[0,55]

    int leftInnerS[4];//����ڲ��α꣬��Χ[0,6],
    int leftInnerR[2];//����ڲ��α꣬��Χ[0,6],

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
            if (leftS[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerS[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerS[i] = 0;
                freeBlockInBuffer(s_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((s_blk[i] = readBlockFromDisk(2000 + leftS[i] / 7 + i * 8 + 16 + 1, buf)) == NULL) {//���¶�һ����
                    perror("Reading Block Failed!\n");
                    return;
                }
            }
            numberS[i] = readNumberFromBlk(s_blk[i], leftInnerS[i] * 8, 0);
        }
        for (int i = 0; i < 2; i++) {
            if (leftR[i] == right)//��ǰ���Ѿ������ֱ꣬����������
                continue;

            if (leftInnerR[i] >= 7) {//�Ѿ���ȡ����һ����
                leftInnerR[i] = 0;
                freeBlockInBuffer(r_blk[i], buf);//�ͷ�֮ǰ�Ŀ�
                if ((r_blk[i] = readBlockFromDisk(2000 + leftR[i] / 7 + i * 8 + 1, buf)) == NULL) {//���¶�һ����
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

        for (int i = 0; i < 4; i++) {//ѡ����С�����Ͷ�Ӧ��ı��
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
        for (int i = 0; i < 2; i++) {//ѡ����С�����Ͷ�Ӧ��ı��
            if (leftR[i] == right) {
                continue;
            }
            if (numberR[i] < minRa) {
                minRa = numberR[i];
                minRb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                minIndexR = i;
            } else if (numberR[i] == minRa) {
                int rb = readNumberFromBlk(r_blk[i], leftInnerR[i] * 8, 4);
                if (rb < minRb) {//�жϵڶ�λ˭��С
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

        if (sc == ra && sd == rb) {//������¼��ͬ��ֻѡȡ һ��

            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd > rb) {//���С�ģ���Ϊsd>rb������r

            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;
        } else if (sc == ra && sd < rb) {//���С�ģ�������s
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
                saveBlock(w_blk, buf, 5300 + write_cnt / 7);
                w_blk = getNewBlockInBuffer(buf);
                memset(w_blk, 0, 64);
            }
            memcpy(w_blk + (write_cnt % 7) * 8, s_blk[minIndexS] + 8 * leftInnerS[minIndexS], 8);
            leftS[minIndexS]++;
            leftInnerS[minIndexS]++;
            write_cnt++;
        } else if (sc > ra) {//���С�ģ�������r
            leftR[minIndexR]++;
            leftInnerR[minIndexR]++;

        } else {//���С�ģ�������s sc<ra
            if (write_cnt % 7 == 0 && write_cnt != 0) {//һ����д���ˣ�����д�ص��ļ� 4000+���.blk
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
        if (memcmp(_target2, leftR, 8) == 0 && memcmp(_target4, leftS, 16) == 0) {//�жϱ���ȫ���ķ���
            break;
        }
    }
    saveBlock(w_blk, buf, 5300 + (write_cnt - 1) / 7 + 1);
    for (int i = 0; i < 4; i++) {//���buffer
        freeBlockInBuffer(s_blk[i], buf);
    }
    for (int i = 0; i < 2; i++) {//���buffer
        freeBlockInBuffer(r_blk[i], buf);
    }

    printf("�������S-R������ %d �����\n�����д���ļ� %d.blk---%d.blk\n", write_cnt, 5301, 5300 + (write_cnt - 1) / 7 + 1);
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
    return myAtoi(string, 4);//��CתΪint��
}

int ShowUserInterface() {
    char input = 0;
    printf("��ѡ�����:\n");
    printf("1.�������������Ĺ�ϵѡ�� : ģ��select S.C, S.D from S where S.C = 50\n");
    printf("2.���׶ζ�·�鲢�����㷨 :\n");
    printf("3.���������Ĺ�ϵѡ���㷨 (��ȷ���Ѷ����ݽ�������)ģ��select S.C, S.D from S where S.C = 50 :\n");
    printf("4.�������������Ĺ�ϵѡ�� (��ȷ���Ѷ����ݽ�������)ģ��select S.C, S.D, R.A, R.B from S inner join R on S.C = R.A��:\n");
    printf("5.���ϲ����㷨:�� S �� R(��ȷ���Ѷ����ݽ�������):\n");
    printf("6.���ϲ����㷨:�� S �� R(��ȷ���Ѷ����ݽ�������):\n");
    printf("7.���ϲ����㷨:�� S - R(��ȷ���Ѷ����ݽ�������):\n");

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
    memcpy(blk + 7 * 8, string_next_blk, 4);//д����һ��blk �ĵ�ַ
    //printf("��д��%d.blk\n",address);
    if (writeBlockToDisk(blk, address, buf) != 0) {
        perror("Writing Block Failed!\n");
    }
}