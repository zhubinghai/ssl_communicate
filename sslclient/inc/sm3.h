#ifndef _SM3_H_
#define _SM3_H_

//定义字寄存器的枚举类型
static const enum
{
	A=0,
	B=1,
	C=2,
	D=3,
	E=4,
	F=5,
	G=6,
	H=7
};

//定义初始值
static const unsigned int IV[8] =
{
	0x7380166F,	0x4914B2B9,
	0x172442D7,	0xDA8A0600,
	0xA96F30BC,	0x163138AA,
	0xE38DEE4D,	0xB0FB0E4E,
};

//定义常量
static unsigned int T[64] = { 0 };

//定义常量
static void _init_T();


/*
    循环左移n比特
    参数：
        @nValue：进行循环左移n比特的值
        @nbit：n比特
    返回值：返回运算结果值
*/
static unsigned int _rotate_left_move(const unsigned int nValue, const unsigned int nBit);


/*
    布尔函数
    参数：
        @X：字寄存器
        @Y：字寄存器
        @Z：字寄存器
    返回值：返回运算结果值，失败返回0
*/
static unsigned int _FF(const unsigned int X,const unsigned int Y,const unsigned int Z,const unsigned int j);

/*
    布尔函数
    参数：
        @X：字寄存器
        @Y：字寄存器
        @Z：字寄存器
    返回值：返回运算结果值，失败返回0
*/
static unsigned int _GG(const unsigned int X, const unsigned int Y, const unsigned int Z, const unsigned int j);


/*
    置换函数
    参数：
        @X：字寄存器
    返回值：返回运算结果值
*/
static unsigned int _P0(const unsigned int X);

/*
    置换函数
    参数：
        @X：字寄存器
    返回值：返回运算结果值
*/
static unsigned int _P1(const unsigned int X);

/*
    消息扩展
    参数：
        @SrcMsgGroup：消息分组数据
        @W68[68]：扩展生成字数据
        @W64[64]：扩展生成字数据
    返回值：无
*/
void msg_exten(unsigned char* SrcMsgGroup,unsigned int W68[68],unsigned int W64[64]);

/*
    压缩函数
    参数：
        @v[8]：存放每次压缩结果
        @SrcMsgGroup:消息分组数据
    返回值：
        @v：压缩结果

*/
static unsigned int* CF(unsigned int v[8],unsigned char* SrcMsgGroup);

/*
    填充消息
    参数：
        @SrcData：原消息数据
        @nSrcLen:消息数据长度
    返回值：
        @MsgBuf：填充后的消息数据
*/
char * msg_fill(unsigned char* SrcData,unsigned int nSrcLen);


/*
    SM3函数
    参数：
        @ucpSrcData：原消息数据
        @nSrcLen:消息数据长度
        @Hash：得到的hash值
    返回值：
        @ucpMsgBuf：填充后的消息数据
*/
extern unsigned int sm3(
	unsigned char* SrcData,
	unsigned int nSrcLen,
	unsigned char* Hash);

#endif
