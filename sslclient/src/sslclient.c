#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "sslclient.h"
#include "sm3.h"

#define MAXBUF 512

/*
    功能：显示证书信息
    参数：
		@ssl：连接后的SSL
	返回值：
		无
*/
void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("数字证书信息:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("证书: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("颁发者: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("无证书信息！\n");
}

/*
    功能：SSL 库初始化
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
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    return ctx;
}

/*
    功能：与服务端进行连接
    参数：
		@ip：服务端ip地址
		@port：服务端端口号
	返回值：
		@sockfd：成功返回创建的套接字描述符
		失败返回-1
*/
int connect_server(const char *ip,short port)
{
    int sockfd=socket(AF_INET,//指定ipv4协议簇
    SOCK_STREAM,//指定流式套接字 tcp
    0);

    if(sockfd==-1)
    {
        perror("socket error");
        return -1;
    }

    int on=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));  //设置套接字属性，允许ip地址重用
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&on,sizeof(on));  //设置套接字属性，允许端口重用

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; //代表ipv4
    serverAddr.sin_port = htons(port); //指定端口号，并进行网络字节序转序
    inet_aton(ip,&(serverAddr.sin_addr));

    int r=connect(sockfd,(struct sockaddr*)&serverAddr,sizeof(serverAddr)); //connect server
    if(r==-1)
    {
        perror("connect fail");
        return -1;
    }

    return sockfd;
}

/*
    功能：发送数据
    参数：
		@ssl：建立ssl连接后的SSL
	返回值：
		成功返回0，失败返回-1
*/
int send_data(SSL *ssl)
{
    char data[MAXBUF]={0};
    scanf("%s",data);
    printf("strlen：%ld\n",strlen(data));
    printf("data：%s\n",data);

    //通过sm3算法获得hash值
    char Hash[33]={0};
    sm3(data,strlen(data),Hash);
    printf("hashlen=%ld\n",strlen(Hash));

    int se=SSL_write(ssl, Hash, 32);

    printf("se=%d\n",se);
    if (se < 0)
    {
        printf("消息发送失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));
        return -1;
    }

    else
    {
         printf("消息发送成功，共发送了%d个字节！\n", se);
    }
    return 0;
}

int main(int argc,char *argv[])
{
    if(argc != 3)
	{
		printf("缺少地址和端口！！！\n");
		return 0;
	}

    SSL_CTX * ctx=ssl_init();   //初始化ssl库

	int sockfd=connect_server(argv[1],atoi(argv[2]));		//连接服务端
	if(sockfd==-1)
	{
        perror("connect fail");
        return 0;
	}

	/* 基于 ctx 产生一个新的 SSL */
    SSL *ssl;
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    /* 建立 SSL 连接 */
    if (SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);
    else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

    }

    ShowCerts(ssl);
    printf("已经连接服务器！！！\n");
    while(1)
    {
        printf("\n");
        printf("-请选择以下功能-\n");
        printf("1.输入字符串，将字符串通过sm3算法，生成的hash值发送给服务端\n");
        printf("0.退出\n");

        char tmp[MAXBUF]={0};
        scanf("%s",tmp);
        getchar();
        if(!strcmp(tmp,"0"))
        {
            SSL_write(ssl, &tmp, 1);
            break;
        }
        else if(!strcmp(tmp,"1"))
        {
            SSL_write(ssl, &tmp, 1);
            printf("请输入字符串\n");
            send_data(ssl);
        }

        else
            ;
    }
    /* 关闭连接 */
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    return 0;
}
