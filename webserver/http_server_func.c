#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <poll.h>
#include <signal.h>

#include <http_serv_common.h>
#include <lib_sock_timeout_func.h>

enum DATA_RECV_TYPE {
	GET,
	POST,
	HEAD,
	UNKNOWN
};

//http接收数据解析结果结构体
typedef	struct
{
	//首先是接收的类型
	enum DATA_RECV_TYPE type;
	//保留
	uint8_t reverse[56];
	//接收的数据实际长度
	int data_len;
	//接收的数据缓冲区
	uint8_t buf[RECV_BUF_SIZE];
} http_data;

typedef	struct
{
	//首先是接收的类型
	enum DATA_RECV_TYPE type;
	//get file path offset in buf
	uint32_t file_path_offset;
	//file path length
	uint32_t file_path_len;
	//保留
	uint8_t reverse[48];
	//接收的数据实际长度
	int data_len;
	//接收的数据缓冲区
	uint8_t buf[RECV_BUF_SIZE];
} http_get_req;

static char *get_send_buf  = NULL;

static void
handle_sigpipe(int sig)
{
	printf("recv sigpipe %d\n",sig);
}

int
init_http_serv_sock(uint16_t serv_port)
{
	signal(SIGPIPE,handle_sigpipe);
	int sock_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sock_fd < 0) {
		perror("socket");
		return sock_fd;
	}
	struct sockaddr_in serv_addr;
	socklen_t serv_addr_len = sizeof(serv_addr);
	bzero(&serv_addr,serv_addr_len);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(serv_port);
	int on = 1;
	if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0) {
		perror("setsockopt");
		close(sock_fd);
		return -1;
	}
	//bind
	if(bind(sock_fd,(struct sockaddr *)(&serv_addr),
				serv_addr_len) < 0) {
		perror("bind");
		close(sock_fd);
		return -1;
	}
	//listen
	if(listen(sock_fd,LISTEN_ACCEPT_MAX) < 0) {
		perror("listen");
		close(sock_fd);
		return -1;
	}
	return sock_fd;
}

void
destroy_http_serv_sock(int sockfd)
{
	if(NULL != s_list) {
		sl_destroy(s_list);
	}
	if(NULL != get_send_buf) {
		free(get_send_buf);
	}
	close(sockfd);
}

//封装一个专门读取http协议的读取方法
//-1表示读取出错0表示连接端可能关闭大于0表示读取成功字节数
static int
read_http_data(int sockfd,http_data *recvinfo)
{
	if(-1 == sockfd) {
		errno = EBADF;
		return -1;
	}
	int recved_len = 0;
	//一行行的读取首先确定请求类型
	//需要对readline的返回0的情况进行仔细的观察，为何这里会影响浏览器访问
	//浏览器第二次读取的时候返回了0说明对方没有发新的数据而是关闭了，为何会这样。
	int ret = readline(sockfd,recvinfo->buf,recvinfo->data_len);
	if(ret < 0) {
		return ret;
	} else if(0 == ret) {
		//client may close
		//DEBUG_PRINT("first read client close\n");
		return ret;
	} else if(0 == strncmp((const char *)recvinfo->buf,"GET ",4)) {//确定接收到的数据类型
		//如果是GET类型获取请求的文件路径，然后进行发送
		//GET后面跟着的就是相对路径和文件
		//接收GET请求头，然后将整个数据进行返回
		recvinfo->type = GET;
		while(recvinfo->buf[0 + recved_len] != '\r' && recvinfo->buf[1 + recved_len] != '\n') {
			//继续接收直到接收所有数据
			if(0 == recved_len) {
				recved_len = ret;
			} else {
				recved_len += ret;
			}
			ret = readline(sockfd,recvinfo->buf + recved_len,recvinfo->data_len - recved_len);
			if(ret <= 0) {
				//DEBUG_PRINT("second read client close\n");
				break;
			}
		}
		return recved_len;
	} else {
		//暂时不处理，直接返回404
		recvinfo->type = UNKNOWN;
		ret = -1;
	}
	return ret;
}

int
analysis_http_data(http_data *recvinfo)
{
	int ret = -1;
	//判断类型
	if(GET == recvinfo->type) {
		http_get_req * grecvinfo = ((http_get_req *)recvinfo);
		//get file path
		grecvinfo->file_path_offset = 4;
		for(grecvinfo->file_path_len = 0;
			' ' != grecvinfo->buf[4 + grecvinfo->file_path_len];
			++grecvinfo->file_path_len);
		//DEBUG_PRINT("%s[%d]\n",grecvinfo->buf + 4,grecvinfo->file_path_len);
		ret = 0;
	}
	return ret;
}


int
send_http_data(int sockfd,http_data *recvinfo)
{
	int ret = -1;
	if(GET == recvinfo->type) {
		http_get_req *grecvinfo = (http_get_req *)recvinfo;
		//本来这个200的响应长度是不定的，http_rsp_200_ext_len是最终结果
		#define	MAX_HTTP_200_EXT_LEN 30
		const ssize_t http_rsp_200_len = strlen(http_rsp_200_def);
		const ssize_t http_rsp_404_len = strlen(http_rsp_404_def);
		const ssize_t max_get_send_len = http_rsp_200_len + \
					MAX_HTTP_200_EXT_LEN + max_file_size;
		ssize_t http_rsp_200_ext_len = 0;
		#undef	MAX_HTTP_200_EXT_LEN
		if(NULL == get_send_buf) {
			get_send_buf = (char *)malloc(max_get_send_len);
			bzero(get_send_buf,max_get_send_len);
			memcpy(get_send_buf,http_rsp_200_def,http_rsp_200_len);
		}
		src_f_list *s_src = NULL;
		//将指定的文件数据发送出去
		for(s_src = s_list;NULL != s_src;s_src = s_src->node) {
			//DEBUG_PRINT("(%d),%s(%d)\n",
				//grecvinfo->file_path_len,s_src->f_path,s_src->f_p_len);

			if(grecvinfo->file_path_len == s_src->f_p_len - 1 && \
				0 == strncmp(grecvinfo->buf + grecvinfo->file_path_offset,
					s_src->f_path,s_src->f_p_len - 1)) {
				break;
			}
		}

		if(NULL != s_src) {
			//DEBUG_PRINT("send src:%s(size:%ld)\n",
					//s_src->f_path,s_src->f_size);
			//send src
			bzero(get_send_buf + http_rsp_200_len,
				max_get_send_len - http_rsp_200_len);
			sprintf(get_send_buf + http_rsp_200_len,
						"%ld\r\n\r\n",s_src->f_size);
			//DEBUG_PRINT("[%s]\n",get_send_buf);
			http_rsp_200_ext_len = strlen(get_send_buf);
			memcpy(get_send_buf + http_rsp_200_ext_len,
						s_src->f_data,s_src->f_size);
			//sprintf(get_send_buf + http_rsp_200_ext_len + s_src->f_size,"\r\n\r\n");
			//int send_len = strlen(get_send_buf);
			int send_len = http_rsp_200_ext_len + s_src->f_size;
			ret = writen(sockfd,get_send_buf,send_len);
			//DEBUG_PRINT("200 writen %s %d\n",s_src->f_path,ret);
		} else {
			//404 not found
			DEBUG_PRINT("we can't find req info %s\n",grecvinfo->buf);
			ret = writen(sockfd,http_rsp_404_def,http_rsp_404_len);
			//DEBUG_PRINT("404 writen %s %d\n",s_src->f_data,ret);
		}
	}
	return ret;
}

inline int
reset_fd_set_and_maxfd(int maxfd_assume,int maxinx,
						int *client,fd_set *rfset)
{
	int maxfd = maxfd_assume;
	FD_ZERO(rfset);
	FD_SET(maxfd,rfset);
	int j;
	for(j = 0;j < maxinx;++j) {
		if(client[j] < 0) {
			continue;
		}
		FD_SET(client[j],rfset);
		if(client[j] > maxfd) {
			maxfd = client[j];
		}
	}
	//printf("reset select fd set and maxfd.\n");
	return maxfd;
}

//select方式来实现IO复用处理发送
//这个select主要坑的地方在于每次select的文件描述符集合收到事件解除阻塞然后进入到业务处理函数中处理完成后，
//必须要重新初始化一边文件描述符集合，然后把要监控的IO重新加入一遍。
//否则文件描述符集合不会再接收新的信号
//上述原因正是导致前期实现的时候总是出现select阻塞导致客户端老是收不到数据的原因。
void
http_server_main_loop_select(int listenedfd)
{
	//select方式来实现IO复用处理发送
	//首先是设定好一个文件描述符的集合，管理收到数据的文件描述符
	fd_set rfset;
	FD_ZERO(&rfset);
	FD_SET(listenedfd,&rfset);

	int nready = 0;
	int maxfd = listenedfd;
	int connfd = -1;
	int client[MAX_FD_SETSIZE] = {[0 ... MAX_FD_SETSIZE - 1] = -1};
	int i = 0;
	int maxinx = 1;
	int ret = 0;

	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);

	http_data h_info;
	while(1) {
		//DEBUG_PRINT("will select!maxfd %d\n",maxfd);
		nready = select(maxfd + 1,&rfset,NULL,NULL,NULL);
		//DEBUG_PRINT("after select!\n");
		if(nready < 0) {
			perror("select");
			break;
		}
		if(nready == 0) {
			continue;
		}
		//否则就是接收到了数据需要进行判断
		if(FD_ISSET(listenedfd,&rfset)) {
			peerlen = sizeof(peeraddr);
			connfd = accept(listenedfd,
				(struct sockaddr *)&peeraddr,&peerlen);
			if(connfd < 0) {
				perror("accept");
				break;
			}
			DEBUG_PRINT("ip = %s port = %d\n",
				inet_ntoa(peeraddr.sin_addr),
					ntohs(peeraddr.sin_port));
			//接收到了新的连接套接字就要将其加入到文件集合中进行监听
			FD_SET(connfd,&rfset);
			//
			for(i = 0;i < MAX_FD_SETSIZE;++i) {
				if(-1 == client[i]) {
					client[i] = connfd;
					if(i >= maxinx) {
						maxinx = i + 1;
					}
					break;
				}
			}
			if(MAX_FD_SETSIZE == i) {
				fprintf(stderr,"too many clients\n");
				exit(EXIT_FAILURE);
			}
			if(connfd > maxfd) {
				maxfd = connfd;
			}
			//说明不需要处理了，因为已经处理完了
			if(--nready <= 0) {
				//重新进入select之前一定要保证文件描述符集合被重新初始化
				maxfd = reset_fd_set_and_maxfd(listenedfd,maxinx,client,&rfset);
				continue;
			}
		}
		//循环判断连接套接字是否有数据收到
		for(i = 0;i < maxinx;++i) {
			if(client[i] < 0) {
				continue;
			}
			if(FD_ISSET(client[i],&rfset)) {
				bzero(&h_info,sizeof(h_info));
				h_info.data_len = sizeof(h_info.buf);
				//处理接收到的数据
				ret = read_http_data(client[i],&h_info);
				if(ret < 0) {
					perror("read_http_data");
					return;
				}
				if(0 == ret) {
					//client close
					DEBUG_PRINT("client close\n");
					//FD_CLR(client[i],&rfset);
					close(client[i]);
					client[i] = -1;
				} else {
					//解析数据填充数据类型结构体
					ret = analysis_http_data(&h_info);
					ret = send_http_data(client[i],&h_info);
					if(ret < 0) {
						perror("send_http_data");
						return;
					}
				}
				
				if(--nready <= 0) {
					break;
				}
			}
		}
		maxfd = reset_fd_set_and_maxfd(listenedfd,maxinx,client,&rfset);
	}
}

void
http_server_main_loop_poll(int listenedfd)
{
	//
	struct pollfd client[MAX_FD_SETSIZE];
	int i = 0,maxinx = 1;
	for(i = 0;i < MAX_FD_SETSIZE;++i) {
		client[i].fd = -1;
	}
	int nready = 0;
	int connfd = -1;
	int ret = 0;
	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);

	http_data h_info;

	client[0].fd = listenedfd;
	client[0].events = POLLIN;
	while(1) {
		nready = poll(client,maxinx,-1);
		if(-1 == nready) {
			if(EINTR == errno) {
				continue;
			}
			perror("poll");
			break;
		}
		if(0 == nready) {
			continue;
		}
		if(client[0].revents & POLLIN) {
			//表明监听套接字有可读的事件
			peerlen = sizeof(peeraddr);
			connfd = accept(listenedfd,
				(struct sockaddr *)&peeraddr,
							&peerlen);
			if(-1 == connfd) {
				perror("accept");
				break;
			}
			for(i = 0;i < MAX_FD_SETSIZE;++i) {
				if(client[i].fd < 0) {
					client[i].fd = connfd;
					if(i > maxinx) {
						maxinx = i + 2;
					} else if(1 == maxinx) {
						maxinx = 2;
					}
					break;
				}
			}
			if(MAX_FD_SETSIZE == i) {
				fprintf(stderr,"too many clients\n");
				exit(EXIT_FAILURE);
			}
			DEBUG_PRINT("ip = %s port = %d\n",
				inet_ntoa(peeraddr.sin_addr),
				ntohs(peeraddr.sin_port));
			//设置好这个套接字要关心的事件
			client[i].events = POLLIN;
			if(--nready <= 0) {
				continue;
			}
		}
		for(i = 1;i < maxinx;++i) {
			//遍历其它的套接字的事件接收情况
			if(-1 == client[i].fd) {
				continue;
			}
			if(client[i].events & POLLIN) {
				//业务代码处理
				bzero(&h_info,sizeof(h_info));
				h_info.data_len = sizeof(h_info.buf);
				//处理接收到的数据
				ret = read_http_data(client[i].fd,&h_info);
				if(ret < 0) {
					perror("read_http_data");
					return;
				}
				if(0 == ret) {
					//client close
					DEBUG_PRINT("client close\n");
					//FD_CLR(client[i],&rfset);
					close(client[i].fd);
					client[i].fd = -1;
				} else {
					//解析数据填充数据类型结构体
					ret = analysis_http_data(&h_info);
					ret = send_http_data(client[i].fd,&h_info);
					if(ret < 0) {
						perror("send_http_data");
						return;
					}
				}
			}
			if(--nready <= 0) {
				break;
			}
		}
	}
}

void
http_server_main_loop_epoll_et(int listenedfd)
{
	int i = 0;
	int nready = 0;
	int connfd = -1;
	int ret = 0;
	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	http_data h_info;

	//建立epoll机制
	int epollfd = epoll_create1(EPOLL_CLOEXEC);
	struct epoll_event event;
	//采用et模式的时候所有加入epoll的套接字都要采用非阻塞的方式
	//如果文件描述符是阻塞的，那么读或者写操作会因为没有后续的事
	//件而一直处于阻塞状态（饥渴状态）
	activate_nonblock(listenedfd);
	event.data.fd = listenedfd;
	event.events = EPOLLIN | EPOLLET;//采用边沿触发的方式
	epoll_ctl(epollfd,EPOLL_CTL_ADD,listenedfd,&event);
	struct epoll_event events[MAX_FD_SETSIZE];
	while(1) {
		nready = epoll_wait(epollfd,events,MAX_FD_SETSIZE,-1);
		if(-1 == nready) {
			if(EINTR == errno) {
				continue;
			}
			perror("epoll_wait");
			break;
		}
		if(0 == nready) {
			continue;
		}

		//每次返回的事件都会保存到events数组中所以可以直接来进行判断
		for(i = 0;i < nready;++i) {
			if(events[i].data.fd == listenedfd) {
				while(1) {
					//accept可能会有多次连接（特别是在进行并发测试的时候）
					//如果不加，可能会导致write出现EPIPE的错误可能是由于
					//epoll的ET模式的机制，如果同时到达的accept不能处理
					//完，那么有些就会饿死在epoll中
					peerlen = sizeof(peeraddr);
					connfd = accept(listenedfd,
						(struct sockaddr *)&peeraddr,&peerlen);
					if(connfd < 0) {
						if(EAGAIN == errno || EWOULDBLOCK == errno) {
							break;
						} else {
							perror("accept");
							return;
						}
					}
					DEBUG_PRINT("ip = %s port = %d\n",
						inet_ntoa(peeraddr.sin_addr),
						ntohs(peeraddr.sin_port));
					//设定为非阻塞套接字
					activate_nonblock(connfd);
					event.data.fd = connfd;
					event.events = EPOLLIN | EPOLLET;
					epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&event);
				}
			} else if(events[i].events & EPOLLIN) {
				connfd = events[i].data.fd;
				if(connfd < 0) {
					continue;
				}
				//业务逻辑代码
				bzero(&h_info,sizeof(h_info));
				h_info.data_len = sizeof(h_info.buf);
				//处理接收到的数据
				ret = read_http_data(connfd,&h_info);
				if(ret < 0) {
					if(EAGAIN == errno || EWOULDBLOCK == errno) {
						continue;
					} else {
						perror("read_http_data");
						return;
					}
				}
				if(0 == ret) {
					//client close
					DEBUG_PRINT("client close\n");
					//FD_CLR(client[i],&rfset);
					close(connfd);
					//从epoll中删除代码
					event = events[i];
					epoll_ctl(epollfd,EPOLL_CTL_DEL,connfd,&event);
					continue;
				}
				//解析数据填充数据类型结构体
				ret = analysis_http_data(&h_info);
				ret = send_http_data(connfd,&h_info);
				if(ret < 0) {
					//这里一般来说会有EPIPE的错误，但是在其它很多的程序中，
					//对这个错误似乎是无法接受的直接报错退出
					perror("send_http_data");
					return;
				}
			}
		}
	}
}
