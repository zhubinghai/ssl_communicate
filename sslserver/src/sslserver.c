#include<stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>

#define MAXBUF 512
#define MAXCLIENT 2

pthread_mutex_t mutex;//定义一个线程互斥锁
static int sockfd=0;
static int client_count=0;

struct comm_para // 定义线程参数结构体
{
    SSL_CTX *ctx;
    int connfd;
    struct sockaddr_in clientAddr;
    int client_num;
};

void sigfun(int sigval)
{
	switch(sigval)
	{
		case SIGINT:	//从键盘中断
            close(sockfd);  //关闭套接字
			break;
		case SIGSEGV:	//无效的内存引用
			printf("年轻人，吃我闪电五连鞭\n");
			break;
		case SIGFPE:	//浮点异常	比如 除数为0
			printf("年轻人，不讲武德，我劝你耗子尾汁\n");
			break;
		case SIGQUIT:	//浮点异常	比如 除数为0
			printf("来偷袭，来骗，我69岁的老同志\n");
			break;
		default:
			printf("大意了，我没有闪\n");
			break;
	}
}


/*
	功能：进行SSL初始化
	参数：
		无
	返回值：
		@ctx：返回创建的SSL_CTX
*/
SSL_CTX * ssl_init()
{
    SSL_CTX *ctx;

    /* SSL 库初始化 */
    if(SSL_library_init() == 0)
        printf("SSL_library_init fail\n");


    /* 载入所有 SSL 算法 */
    if(OpenSSL_add_all_algorithms() == 0)
        printf("OpenSSL_add_all_algorithms fail\n");

    /* 载入所有 SSL 错误消息 */
    SSL_load_error_strings();

   /* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_server_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    //SSL_CTX_set_security_level(ctx, 0);

    char cert[50]="/usr/lib/ssl/misc/cacert.pem";
    char key[50]="/usr/lib/ssl/misc/privkey.pem";
    /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
    if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
        printf("6666\n");
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /* 载入用户私钥 */
    if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* 检查用户私钥是否正确 */
    if (!SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    return ctx;
}

/*
	功能：初始化服务端套接字
	参数：
		@ip：服务端ip地址
		@port：服务端端口号
	返回值：
		@sockfd：成功返回创建的套接字描述符
		失败返回-1
*/
int initTcpsock(const char *ip,short port)
{
	int sockfd = socket(AF_INET, //指定协议族 ：IPV4协议族
		SOCK_STREAM,//指定套接字类型：流式套接字（适用于tcp）
		0//指定应用层协议，一般为0，表示不知名的协议
    );
	if(sockfd == -1)
	{
		perror("socket error");
		return -1;
	}

	//允许端口和ip地址重用
	int on = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	on = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&on,sizeof(on));

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;    //代表我是ipv4协议族
	serverAddr.sin_port = htons(port);  //指定端口号，并且要进行网络字节序转序
	inet_aton(ip,&(serverAddr.sin_addr));   //指定IP地址

	int r = bind(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr));		//绑定
	if(r == -1)
	{
		perror("bind error");
		return -1;
	}

	r = listen(sockfd,3);	//监听
	if(r == -1)
	{
		perror("listen error");
		return -1;
	}

	return sockfd;

}

/*
	功能：接受客户端通过ssl加密后发来的消息
	参数：
		@ssl：建立连接的SSL
	返回值：
		@sockfd：返回创建的套接字描述符
*/
int recv_data(SSL * ssl)
{
    char data[MAXBUF];
    int r=SSL_read(ssl, data, MAXBUF);	//接收客户端发送的报文
    if (r <= 0)
        printf("消息接收失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));

    printf("r=%d\n",r);
    printf("hash: \n");
    for(int i=0;i<r;i++)
    {
        printf("%02X ",data[i]);
    }
      printf("\n");
    return 0;
}

/*
	功能：与客户端进行通信
	参数：
		@connfd：连接成功的套接字描述符
		@clientAddr：客户端的网络地址
	返回值：
		成功返回0
		失败返回-1
*/
int communicate(SSL * ssl,struct sockaddr_in clientAddr)
{
	char cmd[MAXBUF]={0};
	while(1)
	{
        printf("等待客户端发送命令\n");
        int r=SSL_read(ssl, cmd, MAXBUF-1);
		if(r==-1)
		{
			perror("recv fail");
            pthread_mutex_lock(&mutex);
            client_count--;
            pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
			//return -1;
		}

		switch(cmd[0])		//对消息进行解析
		{
			case '0':
				printf("客户端%s请求断开连接\n",inet_ntoa(clientAddr.sin_addr));
				printf("客户端%s已断开\n",inet_ntoa(clientAddr.sin_addr));
				return 0;
			case '1':
				//接收客户端数据
				printf("正在接受客户端%s的数据\n",inet_ntoa(clientAddr.sin_addr));
				recv_data(ssl);
				break;
		}
		printf("\n");
	}
	return 0;
}

void * func(void * arg)
{

	pthread_detach(pthread_self());     //自动回收资源
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL); //允许被取消

    struct comm_para *para=(struct comm_para *)arg;

    /* 基于 ctx 产生一个新的 SSL */
    SSL *ssl;
    ssl = SSL_new(para->ctx);

    /* 将连接用户的 socket 加入到 SSL */
    SSL_set_fd(ssl, para->connfd);

    /* 建立 SSL 连接 */
    int sl=SSL_accept(ssl);
    if (sl <= 0) {
        ERR_print_errors_fp(stdout);
        close(para->connfd);
        return NULL;
    }

    pthread_mutex_lock(&mutex);
    client_count++;
    printf("已经有%d个客户端登入\n",client_count);
    pthread_mutex_unlock(&mutex);

    printf("与我新建立连接的客户端ip为：%s\n",inet_ntoa(para->clientAddr.sin_addr));
    printf("\n");

	communicate(ssl,para->clientAddr);  //进行通信

    SSL_shutdown(ssl);  // 关闭线程的SSL 连接
    SSL_free(ssl);  // 释放线程的SSL
    close(para->connfd);  //关闭线程的connfd

    pthread_mutex_lock(&mutex);
    client_count--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}


int main(int argc,char *argv[])
{
	if(argc != 3)
	{
        printf("缺少地址和端口！！！\n");
		return 0;
	}

	SSL_CTX *ctx=ssl_init();    //初始化SSL库
    printf("ssl_init well\n");

	sockfd=initTcpsock(argv[1],atoi(argv[2]));	//初始化套接字
	signal(SIGINT,sigfun);

    pthread_t tid[MAXCLIENT];
    pthread_mutex_init(&mutex,NULL);//初始化线程互斥锁
	while(1)
	{

		struct sockaddr_in clientAddr;
		socklen_t  addrlen = sizeof(clientAddr);
		printf("\n");
		printf("等待连接\n");
		int connfd = accept(sockfd,(struct sockaddr *)&clientAddr,&addrlen);	//等待连接
		if(connfd == -1)
		{
			perror("connect fail");
			break;
		}

        pthread_mutex_lock(&mutex);
        if(client_count>=MAXCLIENT)
        {
            printf("客户端太多，服务器繁忙!!!\n");
            close(connfd);  //关闭当前连接的connfd
            pthread_mutex_unlock(&mutex);
            continue;
        }
        pthread_mutex_unlock(&mutex);

        struct comm_para para;
        para.ctx=ctx;
        para.connfd=connfd;
        para.clientAddr=clientAddr;

        pthread_mutex_lock(&mutex);
        pthread_create(&tid[client_count],NULL,func,(void *)&para); //创建线程用于于客户端通信
        pthread_mutex_unlock(&mutex);
	}

    pthread_mutex_lock(&mutex);
	for(int i=0;i<client_count;i++)
        pthread_cancel(tid[i]); //取消所有线程
	for(int i=0;i<client_count;i++)
        pthread_join(tid[i],NULL);  //等待所有线程释放资源
    pthread_mutex_unlock(&mutex);

    SSL_CTX_free(ctx);  // 释放CTX
    pthread_mutex_destroy(&mutex);
    printf("已退出!!!\n");
    return 0;

}
