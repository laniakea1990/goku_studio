#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <lib_sock_timeout_func.h>

#define	ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)

int
read_timeout(int sockfd,int timeval)
{
	int ret = 0;
	//首先就是判断超时时间
	if(timeval > 0) {
		//首先还是设置文件描述符集合
		fd_set fs;
		FD_ZERO(&fs);
		FD_SET(sockfd,&fs);

		//设定好超时时间
		struct timeval timeout;
		timeout.tv_sec = timeval;
		timeout.tv_usec = 0;
		//调用select进行阻塞
		do {
			ret = select(sockfd + 1,&fs,NULL,NULL,&timeout);
		} while(-1 == ret && EINTR == errno);
		//分情况进行处理
		if(ret < 0) {
			//说明发生了其它的错误
			return ret;
		} else if(0 == ret) {
			//说明超时了，需要进行返回
			errno = ETIMEDOUT;
			ret = -1;
		} else {
			//说明没有超时，有数据到了，需要返回
			ret = 0;
		}
	}
	return ret;
}

int
write_timeout(int sockfd,int tval)
{
	int ret = 0;
	if(tval > 0) {
		fd_set fs;
		FD_ZERO(&fs);
		FD_SET(sockfd,&fs);

		struct timeval timeout;
		timeout.tv_sec = tval;
		timeout.tv_usec = 0;

		do {
			ret = select(sockfd + 1,NULL,&fs,NULL,&timeout);
		} while(-1 == ret && EINTR == errno);
		if(-1 == ret) {
			return ret;
		} else if(0 == ret) {
			errno = ETIMEDOUT;
			ret = -1;
		} else {
			ret = 0;
		}
	}
	return ret;
}

int
accept_timeout(int sockfd,struct sockaddr *addr,
		socklen_t *addrlen,int tval)
{
	int ret = 0;
	if(tval > 0) {
		fd_set fs;
		FD_ZERO(&fs);
		FD_SET(sockfd,&fs);

		struct timeval timeout;
		timeout.tv_sec = tval;
		timeout.tv_usec = 0;

		do {
			//这里是可读文件的描述符的集合
			ret = select(sockfd + 1,&fs,NULL,NULL,&timeout);
		} while(-1 == ret && EINTR == errno);
		if(-1 == ret) {
			return ret;
		} else if(0 == ret) {
			errno = ETIMEDOUT;
			ret = -1;
		} else {
			//因为这里是监听套接字，当套接字接收到连接后
			//就可以调用accept了
			ret = accept(sockfd,addr,addrlen);
		}
	}
	return ret;
}

void
activate_nonblock(int sockfd)
{
	int ret;
	int flags = fcntl(sockfd,F_GETFL);
	if(-1 == flags) {
		ERR_EXIT("fcntl");
	}
	flags |= O_NONBLOCK;
	ret = fcntl(sockfd,F_SETFL,flags);
	if(-1 == ret) {
		ERR_EXIT("fcntl");
	}
}

void
deactivate_nonblock(int sockfd)
{
	int ret;
	int flags = fcntl(sockfd,F_GETFL);
	if(-1 == flags) {
		ERR_EXIT("fcntl");
	}
	flags &= ~O_NONBLOCK;
	ret = fcntl(sockfd,F_SETFL,flags);
	if(-1 == ret) {
		ERR_EXIT("fcntl");
	}
}

int
connect_timeout(int sockfd,const struct sockaddr *addr,
			socklen_t addrlen,int tval)
{
	int ret = 0;
	if(tval > 0) {
		//这里首先进行connect操作，然后是
		//定时看连接是否超时，如果不超时就可以写了
		//所以不能阻塞到连接成功，
		//而应该通过select来定时判断是否连接成功
		activate_nonblock(sockfd);
	}
	ret = connect(sockfd,(struct sockaddr *)addr,addrlen);
	if(ret < 0 && EINPROGRESS == errno) {
		fd_set connect_fdset;
		struct timeval timeout;
		timeout.tv_sec = tval;
		timeout.tv_usec = 0;
		FD_ZERO(&connect_fdset);
		FD_SET(sockfd,&connect_fdset);
		do {
			ret = select(sockfd + 1,NULL,&connect_fdset,NULL,&timeout);
		} while(ret < 0 && EINTR == errno);
		if(0 == ret) {
			//timeout
			errno = ETIMEDOUT;
			ret = -1;
		} else if(ret < 0) {
			return ret;
		} else {
			//这里返回有两种情况，要么是连接成功，要么连接失败，
			//但是连接失败不会在select中以错误的方式返回
			//所以需要再次使用getsockopt来进行近一步的判断
			int err;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&err,&socklen);
			if(-1 == sockoptret) {
				return sockoptret;
			}
			if(0 == err) {
				ret = 0;
			} else {
				errno = err;
				ret = -1;
			}
		}
	}
	//要恢复阻塞的状态
	if(tval > 0) {
		deactivate_nonblock(sockfd);
	}
	return ret;
}

ssize_t
readn(int fp,void *buf,size_t count)
{
	int ret = 0;
	//这里按照read调用的方式来封转
	size_t nleft = count;
	ssize_t nread;

	char *bufp = (char *)buf;
	while(nleft > 0) {
		//进行超时判断
		if((ret = read_timeout(fp,5)) < 0) {
			//timeout
			return ret;
		}
		if((nread = read(fp,bufp,nleft)) < 0) {
			//小于0有几种情况
			if(errno == EINTR) {//这个是被信号中断不认为错
				continue;
			} 
			//其它错误都直接返回失败
			return nread;
		} else if(0 == nread) {
			return count - nleft;//返回当前读取到的实际数据
		}
		//剩下的情况就是正常读取了数据
		bufp += nread;
		nleft -= nread;
	}
	//返回所有数据
	return count;
}

ssize_t
writen(int fp,const void *buf,size_t count)
{
	int ret = 0;
	//这里按照write的调用方式来封转
	size_t nleft = count;//剩余需要发送的字节数
	ssize_t nwrite;//已经成功发送的字节数

	char *bufp = (char *)buf;
	while(nleft > 0) {
		//进行超时判断
		if((ret = write_timeout(fp,5)) < 0) {
			return ret;
		}
		if((nwrite = write(fp,bufp,nleft)) < 0) {
			if(EINTR == errno) {//这个是被信号中断不认为错
				continue;
			}
			//其它错误都直接返回失败
			return nwrite;
		} else if(0 == nwrite) {//说明写入了0个字节等于什么也没发送
			continue;
		}
		//发送了数据
		bufp += nwrite;
		nleft -= nwrite;
	}
	return count;//必须是完全发送完否则都算错
}

ssize_t
recv_peek(int sockfd,void *buf,size_t len)
{
	int ret;
	while(1) {
		ret = recv(sockfd,buf,len,MSG_PEEK);
		if(-1 == ret && EINTR == errno) {
			continue;
		}
		return ret;
	}
}

ssize_t
readline(int sockfd,void *buf,size_t maxline)
{
	int ret,i;
	int nread;
	char *bufp = (char *)buf;
	int nleft = maxline;
	//只要遇到\n就可以了
	while(1) {
		ret = recv_peek(sockfd,bufp,nleft);
		if(ret < 0) {
			return ret;
		} else if(0 == ret) {
			return ret;//说明对等方可能关闭了
		}
		//printf("echoserv recv_peek ret:%d\n",ret);
		nread = ret;
		for(i = 0;i < nread;++i) {
			if(bufp[i] == '\n') {
				//说明读取到了一行
				//然后这里读取的话，由于设置了MSG_PEEK所以数据
				//并没有移除
				ret = readn(sockfd,bufp,i + 1);
				//printf("readn ret %d i + 1 %d\n",ret,i + 1);
				if(ret != i + 1) {
					printf("ret != i + 1\n");
					exit(EXIT_FAILURE);
				}
				return (maxline - nleft) + ret;
			}
		}
		//这里说明没有读取到一行消息，所以需要暂时
		//读出来放到缓冲区当中
		if(nread > nleft) {
			printf("nread(%d) > nleft(%d)\n",nread,nleft);
			exit(EXIT_FAILURE);
		}
		nleft -= nread;
		ret = readn(sockfd,bufp,nread);
		if(ret != nread) {//因为已经明确了有nread长度的数据
			printf("ret != nread");
			exit(EXIT_FAILURE);
		}
		bufp += nread;//继续去看下一个数据直到有一行数据
	}
	return -1;
}
