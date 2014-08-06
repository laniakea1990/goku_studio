#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <ftw.h>
#include <strings.h>
#include <fcntl.h>

#include <http_serv_common.h>

static void
s_list_free(void *node)
{
	src_f_list *tmp = node;
	if(NULL != tmp) {
		free(tmp->f_path);
		free(tmp->f_data);
		free(tmp);
	}
}

static void *
s_list_match(void *list,void *data)
{
	src_f_list *tmp = (src_f_list *)list;
	req_info *d = (req_info *)data;
	if(NULL == d || NULL == d->r_info)
		return NULL;
	if(GET_REQ == d->r_t)
		while(NULL != tmp) {
			if(tmp->f_p_len == d->r_info_len)
				if(0 == strncmp(tmp->f_path,d->r_info,tmp->f_p_len - 1)) {
					//DEBUG_PRINT("tmp->f_path:%s,d->r_info:%s\n",tmp->f_path,d->r_info);
					return tmp;
				}
			tmp = tmp->node;
		}
	return NULL;
}

static int
scan_src_file_cb(const char *fpath,const struct stat *sb,
			int tflag,struct FTW *ftwbuf)
{
    //record file to list
	src_f_list *n_file = NULL,*n_file_old = NULL;
	int fd = -1;
	char *index_str = NULL;
	switch(tflag)
	{
	case FTW_D:
		break;
	case FTW_F:
		n_file = (src_f_list *)malloc(sizeof(src_f_list));
		bzero(n_file,sizeof(src_f_list));
		n_file->node = NULL;
		n_file->f_p_len = strlen(fpath) + 1;
		n_file->f_path = (char *)malloc(n_file->f_p_len);
		bzero(n_file->f_path,n_file->f_p_len);
		memcpy(n_file->f_path,fpath,n_file->f_p_len - 1);
		n_file->req_count = 0;
		n_file->f_size = (intmax_t)(sb->st_size);
		if(max_file_size < n_file->f_size)
			max_file_size = n_file->f_size;
		n_file->f_data = (uint8_t *)malloc(n_file->f_size);
		bzero(n_file->f_data,n_file->f_size);
		if((fd = open(fpath,O_RDONLY,0)) < 0) {
			perror("open");
			return -1;
		}
		read(fd,n_file->f_data,n_file->f_size);
		if(NULL == s_list) {
			s_list = n_file;
			regist_delete_call_back(s_list,s_list_free);
			regist_match_call_back(s_list,s_list_match);
		} else
			sl_node_insert_tail_absolute(s_list,n_file);
		//check the file if a index.html we ignore the case
		index_str = strcasestr(n_file->f_path,"/index.html");
		#define	INDEX_NAME_LEN 11
		if(NULL != index_str && index_str[INDEX_NAME_LEN] == '\0') {
			n_file_old = n_file;
			n_file = (src_f_list *)malloc(sizeof(src_f_list));
			bzero(n_file,sizeof(src_f_list));
			n_file->node = NULL;
			n_file->f_p_len = n_file_old->f_p_len - INDEX_NAME_LEN;
			n_file->f_path = (char *)malloc(n_file->f_p_len);
			bzero(n_file->f_path,n_file->f_p_len);
			memcpy(n_file->f_path,n_file_old->f_path,n_file->f_p_len - 1);
			n_file->req_count = n_file_old->req_count;
			n_file->f_size = n_file_old->f_size;
			n_file->f_data = (uint8_t *)malloc(n_file->f_size);
			bzero(n_file->f_data,n_file->f_size);
			memcpy(n_file->f_data,n_file_old->f_data,n_file->f_size);
			sl_node_insert_tail_absolute(s_list,n_file);
		}
		#undef	INDEX_NAME_LEN
		close(fd);
		break;
	case FTW_DNR:
		break;
	case FTW_NS:
		break;
	case FTW_SL:
		break;
	case FTW_SLN:
		break;
	default:
		break;
	}
	return 0;
}

bool
load_http_server_src(const char *src_path)
{
	if(NULL == src_path)
		return false;
	//process src_path if the string end with
	//'/' we cut it
	uint32_t src_path_len = strlen(src_path);
	if('/' == src_path[src_path_len - 1])
		src_path_len -= 1;
	int flags = FTW_PHYS;
	if(nftw(src_path,scan_src_file_cb,20,flags) < 0) {
		perror("nftw");return false;
	}
	//after dirent scan we get src file list
	//we cut src_path of each file path
	src_f_list *tmp = s_list;
	char *new_f_path = NULL;
	while(NULL != tmp) {
		tmp->f_p_len -= src_path_len;
		if(1 == tmp->f_p_len) {
			tmp->f_p_len = 2;
			new_f_path = (char *)malloc(tmp->f_p_len);
			bzero(new_f_path,tmp->f_p_len);
			new_f_path[0] = '/';
		} else {
			new_f_path = (char *)malloc(tmp->f_p_len);
			bzero(new_f_path,tmp->f_p_len);
			memcpy(new_f_path,tmp->f_path + src_path_len,
							tmp->f_p_len);
		}
		free(tmp->f_path);
		tmp->f_path = new_f_path;
		DEBUG_PRINT("new f_path:%s\n",tmp->f_path);
		tmp = tmp->node;
	}
	return true;
}
