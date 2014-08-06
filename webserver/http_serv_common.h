#ifndef	_HTTP_SERV_COMMON_H_
#define	_HTTP_SERV_COMMON_H_
#include <unistd.h>
#include <sys/socket.h>
#include <list_ex_single.h>

#define LISTEN_ACCEPT_MAX SOMAXCONN
#define RECV_BUF_SIZE 512

#define	MAX_SYM_EP_EVENT 2
#define	PTHREAD_INIT_NUM 20
#define	SIG_PTH_WAKE_UP SIGRTMIN + 1
#define	SIG_PTH_EXIT SIGRTMIN + 2
#define	MAX_FD_SETSIZE FD_SETSIZE

#define DEBUG_PRINT(...) printf(__VA_ARGS__)

//the source file list
typedef struct {
	sl_node *node;
	//f_path len with '\0'
	uint32_t f_p_len;
	//file path name(with
	//file name)
	char *f_path;
	//this file requested count
	uint32_t req_count;
	//file size
	size_t f_size;
	uint8_t *f_data;
} src_f_list;

typedef struct {
	int client_fd;
	sigset_t *s_set;
} rs_pth_arg;

typedef enum {
	NONE_REQ,
	GET_REQ,
	POST_REQ
} r_type;

typedef struct {
	r_type r_t;
	//with '\0';
	uint32_t r_info_len;
	char *r_info;
} req_info;

extern char *http_rsp_200_def;
extern char *http_rsp_404_def;
extern src_f_list *s_list;
extern size_t max_file_size;

#endif
