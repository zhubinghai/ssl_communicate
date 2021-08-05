#ifndef _SSLCLIENT_H_
#define _SSLCLIENT_H_

void ShowCerts(SSL * ssl);

/* SSL 库初始化，参看 ssl-server.c 代码 */
SSL_CTX * ssl_init();

/*
    功能：与服务端进行连接
    参数：
		@ip：服务端ip地址
		@port：服务端端口号
	返回值：
		返回创建的套接字描述符
*/
int connect_server(const char *ip,short port);

/*
    功能：发送数据
    参数：
		@sockfd：套接字描述符
	返回值：
		成功返回0，失败返回-1
*/
int send_data(SSL *ssl);



#endif
