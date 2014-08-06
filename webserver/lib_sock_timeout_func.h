#ifndef	_LIB_SOCK_TIMEOUT_FUNC_H_
#define	_LIB_SOCK_TIMEOUT_FUNC_H_
#include <sys/types.h>
#include <sys/socket.h>

void activate_nonblock(int sockfd);

/**
 * read_timeout
 * 输入参数sock_fd套接字timeval超时时间（秒）
 * 如果超时时间为0则不进行超时
 * 返回-1且errno为ETIMEDOUT为超时否则返回0
 **/
int read_timeout(int sockfd,int timeval);
/**
 * write_timeout
 **/
int write_timeout(int sockfd,int timeval);
/**
 * accept_timeout
 **/
int accept_timeout(int sockfd,struct sockaddr *addr,
		socklen_t *addrlen,int timeval);
/**
 * connect_timeout
 **/
int connect_timeout(int sockfd,const struct sockaddr *addr,
			socklen_t addrlen,int timeval);

ssize_t readn(int fp,void *buf,size_t count);

ssize_t writen(int fp,const void *buf,size_t count);

ssize_t recv_peek(int sockfd,void *buf,size_t len);

ssize_t readline(int sockfd,void *buf,size_t maxline);

#endif

