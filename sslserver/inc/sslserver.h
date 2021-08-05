#ifndef _SSLSERVER_H_
#define _SSLSERVER_H_

/*
	功能：进行SSL初始化
	参数：
		无
	返回值：
		@ctx：返回创建的SSL_CTX
*/
SSL_CTX * ssl_init();

/*
	功能：初始化服务端套接字
	参数：
		@ip：服务端ip地址
		@port：服务端端口号
	返回值：
		@sockfd：成功返回创建的套接字描述符
		失败返回-1
*/
int initTcpsock(const char *ip,short port);

/*
	功能：接受客户端通过ssl加密后发来的消息
	参数：
		@ssl：建立连接的SSL
	返回值：
		@sockfd：返回创建的套接字描述符
*/
int recv_data(SSL * ssl);

/*
	功能：与客户端进行通信
	参数：
		@connfd：连接成功的套接字描述符
		@clientAddr：客户端的网络地址
	返回值：
		成功返回0
		失败返回-1
*/
int communicate(SSL * ssl,struct sockaddr_in clientAddr);


#endif
