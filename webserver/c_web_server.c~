#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <http_serv_common.h>
#include <http_server_func.h>
#include <http_src_func.h>


//usage:c_web_server port src_path
int
main(int argc,char *argv[])
{
	int ret = 0;
	int serv_net_sock = -1;
	//load src
	if(!load_http_server_src(argv[2])) {
		fprintf(stderr,"src file load error!\n");
		ret = -1;
		goto ERR_EXIT;
	}
	DEBUG_PRINT("max file size:%ld\n",max_file_size);
	//init sock
	uint16_t s_port = atol(argv[1]);
	serv_net_sock = init_http_serv_sock(s_port);
	if(serv_net_sock < 0) {
		perror("init_http_serv_sock");
	}
	//http_server_main_loop_select(serv_net_sock);
	//http_server_main_loop_poll(serv_net_sock);
	http_server_main_loop_epoll_et(serv_net_sock);
	destroy_http_serv_sock(serv_net_sock);
ERR_EXIT:
	return ret;
}
