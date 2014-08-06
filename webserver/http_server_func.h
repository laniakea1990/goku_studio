#ifndef	_HTTP_SERVER_FUNC_H_
#define	_HTTP_SERVER_FUNC_H_
#include <stdint.h>

#include <http_serv_common.h>

int init_http_serv_sock(uint16_t serv_port);

void destroy_http_serv_sock(int sockfd);

void http_server_main_loop_select(int listenedfd);

void http_server_main_loop_poll(int listenedfd);

void http_server_main_loop_epoll_et(int listenedfd);

#endif
