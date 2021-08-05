#include "sm3.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sm3.h"

static void _init_T()
{
	int i = 0;
	for (; i < 16; i++)
		T[i] = 0x79CC4519;
	for (; i < 64; i++)
		T[i] = 0x7A879D8A;
}


/*
    循环左移n比特
    参数：
        @nValue：进行循环左移n比特的值
        @nbit：n比特
    返回值：返回运算结果值
*/
static unsigned int _rotate_left_move(const unsigned int nValue, const unsigned int nBit)
{
	return ((nValue << nBit)&0xFFFFFFFF | ((nValue&0xFFFFFFFF) >> (32 - nBit)));
}

/*
    布尔函数
    参数：
        @X：字寄存器
        @Y：字寄存器
        @Z：字寄存器
    返回值：返回运算结果值，失败返回0
*/
static unsigned int _FF(const unsigned int X,const unsigned int Y,const unsigned int Z,const unsigned int j)
{
	if (0 <= j && j < 16)
		return (X ^ Y ^ Z);
	else if (16 <= j && j < 64)
		return ((X & Y) | (X & Z) | (Y & Z));

	return 0;
}

/*
    布尔函数
    参数：
        @X：字寄存器
        @Y：字寄存器
        @Z：字寄存器
    返回值：返回运算结果值，失败返回0
*/
static unsigned int _GG(const unsigned int X, const unsigned int Y, const unsigned int Z, const unsigned int j)
{
	if (0 <= j && j < 16)
		return (X ^ Y ^ Z);
	else if (16 <= j && j < 64)
		return ((X & Y) | ((~X) & Z));

	return 0;
}

/*
    置换函数
    参数：
        @X：字寄存器
    返回值：返回运算结果值
*/
static unsigned int _P0(const unsigned int X)
{
	return (X ^ (_rotate_left_move(X, 9)) ^ (_rotate_left_move(X, 17)));
}

/*
    置换函数
    参数：
        @X：字寄存器
    返回值：返回运算结果值
*/
static unsigned int _P1(const unsigned int X)
{
	return (X ^ (_rotate_left_move(X, 15)) ^ (_rotate_left_move(X, 23)));
}

/*
    消息扩展
    参数：
        @SrcMsgGroup：消息分组数据
        @W68[68]：扩展生成字数据
        @W64[64]：扩展生成字数据
    返回值：无
*/
void msg_exten(unsigned char* SrcMsgGroup,unsigned int W68[68],unsigned int W64[64])
{
	int j = 0;
	for (j = 0; j < 16; j++)
	{
		W68[j] = ((unsigned int)SrcMsgGroup[j * 4 + 0] << 24) & 0xFF000000
			| ((unsigned int)SrcMsgGroup[j * 4 + 1] << 16) & 0x00FF0000
			| ((unsigned int)SrcMsgGroup[j * 4 + 2] << 8) & 0x0000FF00
			| ((unsigned int)SrcMsgGroup[j * 4 + 3] << 0) & 0x000000FF;
	}

	for (j = 16; j < 68; j++)
	{
		W68[j] = _P1(W68[j - 16] ^ W68[j - 9] ^ (_rotate_left_move(W68[j - 3], 15))) ^ (_rotate_left_move(W68[j - 13], 7)) ^ W68[j - 6];
	}

	for (j = 0; j < 64; j++)
	{
		W64[j] = W68[j] ^ W68[j + 4];
	}
}

/*
    压缩函数
    参数：
        @v[8]：存放每次压缩结果
        @SrcMsgGroup:消息分组数据
    返回值：
        @v：压缩结果

*/
static unsigned int* CF(unsigned int v[8],unsigned char* SrcMsgGroup)
{

    unsigned int W68[68] = { 0 };
	unsigned int W64[64] = { 0 };

    msg_exten(SrcMsgGroup,W68,W64);  //消息扩展

    unsigned int A_G[8] = { 0 };    //定义八个字寄存器存放每次压缩后的结果
	for (int i= 0; i < 8; i++)
	{
		A_G[i] = v[i];
	}

    //tempporary variable
	unsigned int SS1 = 0, SS2 = 0, TT1 = 0, TT2 = 0;

	for (int j = 0; j < 64; j++)
	{
		SS1 = _rotate_left_move((_rotate_left_move(A_G[A], 12) + A_G[E] + _rotate_left_move(T[j], j % 32)), 7);
		SS2 = SS1 ^ (_rotate_left_move(A_G[A], 12));
		TT1 = _FF(A_G[A], A_G[B], A_G[C], j) + A_G[D] + SS2 + W64[j];
		TT2 = _GG(A_G[E], A_G[F], A_G[G], j) + A_G[H] + SS1 + W68[j];
		A_G[D] = A_G[C];
		A_G[C] = _rotate_left_move(A_G[B], 9);
		A_G[B] = A_G[A];
		A_G[A] = TT1;
		A_G[H] = A_G[G];
		A_G[G] = _rotate_left_move(A_G[F], 19);
		A_G[F] = A_G[E];
		A_G[E] = _P0(TT2);
	}

	for (int i = 0;i < 8; i++)
	{
		v[i] = A_G[i] ^ v[i];
	}

	return v;
}


/*
    填充消息
    参数：
        @SrcData：原消息数据
        @nSrcLen:消息数据长度
    返回值：
        @MsgBuf：填充后的消息数据
*/
char * msg_fill(unsigned char* SrcData,unsigned int nSrcLen)
{
    unsigned int nGroupNum=(nSrcLen + 1 + 8) / 64 +1 ;  //计算出填充后所需几个64字节（512比特）
    unsigned char *MsgBuf = (unsigned char*)malloc(nGroupNum * 64);  //分配填充后所需空间
    memset(MsgBuf, 0, nGroupNum * 64);   //将填充后的数组全清空置0
	memcpy(MsgBuf, SrcData, nSrcLen); //将原始消息的内容复制过来
	MsgBuf[nSrcLen] = 0x80;  //将消息数据的后一位置1

    for(int i=0;i<8;i++)
    {
        //将消息长度的8字节数据存放在填充消息的后8字节
        MsgBuf[nGroupNum * 64 - i - 1] = (MsgBuf[nGroupNum * 64 - i - 1]) | ((unsigned long long)(nSrcLen * 8) >> (i * 8));
    }

    return MsgBuf;
}


/*
    SM3函数
    参数：
        @ucpSrcData：原消息数据
        @nSrcLen:消息数据长度
        @Hash：得到的hash值
    返回值：
        @ucpMsgBuf：填充后的消息数据
*/
unsigned int sm3(
	unsigned char* SrcData,
	unsigned int nSrcLen,
	unsigned char* Hash)
{

	_init_T();  //定义常量


    char *MsgBuf = msg_fill(SrcData,nSrcLen);   //消息填充


    unsigned int nGroupNum=(nSrcLen + 1 + 8) / 64 +1 ;  //计算出填充后所需几个64字节（512比特）


    int **v=(int **)malloc(sizeof(int *)*(nGroupNum+1));    //分配空间用于存放每次压缩结果
    for(int i=0;i<nGroupNum+1;i++)
    {
        v[i]=(int *)malloc(sizeof(int)*8);
    }


	for (int i = 0; i < 8; i++) //存放IV初始值
	{
		v[0][i] = IV[i];
	}

	for (int i = 0; i < nGroupNum; i++) //迭代压缩函数
		v[i+1]=CF(v[i],&MsgBuf[i * 64]);


	free(MsgBuf);
	//释放v


	//得到hash结果
	for (int i = 0; i < 8; i++)
	{
		Hash[i * 4 + 0] = (unsigned char)((v[nGroupNum][i] >> 24) & 0xFF);
		Hash[i * 4 + 1] = (unsigned char)((v[nGroupNum][i] >> 16) & 0xFF);
		Hash[i * 4 + 2] = (unsigned char)((v[nGroupNum][i] >>  8) & 0xFF);
		Hash[i * 4 + 3] = (unsigned char)((v[nGroupNum][i] >>  0) & 0xFF);
	}

	return 0;
}
/*
int main()
{
	//char SrcData[4]="abc";
	char SrcData[65]={0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,
0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64,0X61,0X62,0X63,0X64};

	char Hash[33];

    sm3(SrcData,strlen(SrcData),Hash);

	for(int i=0;i<32;i++)
	{
        printf("%02X ",Hash[i]);
	}
	printf("\n");

	printf("SrcData: %s\n",SrcData);
	printf("Hash: %s\n",Hash);
	return 0;
}
*/
