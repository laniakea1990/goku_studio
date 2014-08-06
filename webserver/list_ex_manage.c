/*
 * list_ex_manage.c
 *
 *  Created on: 2011-12-22
 *      Author: mobilefzb
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
//#include <sys_ex/stdio_ex.h>
#include <list_ex_single.h>
#include <list_ex_manage.h>

//删除管理链表
static call_back_manage *delete_call_back_list = NULL;
//删除回调节点，用于保存最近删除操作中调用的回调
static call_back_manage used_del_cb = {
	NULL,NULL,{NULL}
};
//匹配管理链表
static call_back_manage *match_call_back_list = NULL;
//匹配回调节点，用于保存最近匹配操作中调用的回调
static call_back_manage used_match_cb = {
	NULL,NULL,{NULL}
};

//这里链表管理完全没有使用统一的接口进行管理
//主要由于统一接口中的的回调功能导致性能下降
//比较严重权衡下对于速度要求低但是稳定性以及
//开发周期短这些要求高的可以采用统一接口进行
//开发，一些大数据处理的方法最好是自己实现相
//关处理
static void *
delete_manage_call_back_del(void *list,void *del_node)
{
	if(NULL == del_node || NULL == list)
		return list;
	if(list == del_node)
		list = ((call_back_manage *)list)->node;
	else {
		call_back_manage *tmp = list;
		while(tmp->node != NULL && tmp->node != del_node)
			tmp = tmp->node;
		if(tmp->node == del_node)
			tmp->node = ((call_back_manage *)del_node)->node;
	}
	free(del_node);
	return list;
}

static void *
delete_manage_call_back_match(void *list,void *data)
{
	while(NULL != list && ((call_back_manage *)list)->list_head_addr != data)
		list = ((call_back_manage *)list)->node;
	return list;
}

//对于最基本的管理函数，还是不使用统一接口比较好
//否则性能损失比较大，不过当链表数量特别多的时候
//使用统一的接口的好处就是可以简化劳动强度提高代
//码的可读性和可维护性


//获取指定链表的删除回调
void
(*get_delete_call_back(void *reg_list_h))(void *)
{
	//调用管理链表获取该链表
	if(NULL == delete_call_back_list) {
		fprintf(stderr,"error happened in %s,because no call back regist",__func__);
		abort();
	}
	call_back_manage *temp = delete_call_back_list;
	if(reg_list_h == used_del_cb.list_head_addr)
		return used_del_cb.call_back.usr_delete_call_back;
	//统一接口最后的结果就是找到这样的函数来调用
	//这里为了提高性能就直接使用
	temp = delete_manage_call_back_match(delete_call_back_list,reg_list_h);
	if(NULL == temp) {
		fprintf(stderr,"error happened in %s,because no call back found for this list",__func__);
		abort();
	}
	used_del_cb.list_head_addr = temp->list_head_addr;
	used_del_cb.call_back.usr_delete_call_back = temp->call_back.usr_delete_call_back;

	return temp->call_back.usr_delete_call_back;
}

//检查指定链表是否注册删除回调
bool
check_delete_regist_state(void *reg_list_h)
{
	//检查链表，检查到就返回
	if(NULL == delete_call_back_list)
		return false;//这种情况肯定没有任何函数注册
	call_back_manage *temp = delete_call_back_list;//获取链表
	if(reg_list_h == used_del_cb.list_head_addr)
			return true;//先检测记录的缓存
	temp = delete_manage_call_back_match(delete_call_back_list,reg_list_h);
	if(NULL == temp)
		return false;//说明实在没有
	return true;//说明有注册的情况
}

bool
regist_delete_call_back(void *reg_list_h,
			void (*usr_delete_call_back)(void *))
{
	//其他的链表都在这里注册回调函数
	//在正式注册之前需要确保这个链表
	//在管理链表中没有注册回调
	unregist_delete_call_back(reg_list_h);
	call_back_manage *new = (call_back_manage *)malloc(sizeof(call_back_manage));
	new->node = NULL;
	new->list_head_addr = reg_list_h;
	new->call_back.usr_delete_call_back = \
		usr_delete_call_back;
	//进行链表连接
	if(NULL == delete_call_back_list)
		delete_call_back_list = new;
	else
		sl_insert_tail_relative(delete_call_back_list,new);
	return true;
}

bool
unregist_delete_call_back(void *reg_list_h)
{
	if(NULL == delete_call_back_list)
		return false;
	call_back_manage *temp = delete_call_back_list;
	//找到匹配的节点
	temp = delete_manage_call_back_match(delete_call_back_list,reg_list_h);
	if(NULL == temp)
		return false;

	delete_call_back_list = delete_manage_call_back_del(delete_call_back_list,temp);
	//这里还要处理一下当前用过的回调
	//记录变量，防止被卸载的回调又用
	//到该链表中
	if(used_del_cb.list_head_addr == reg_list_h) {
		used_del_cb.list_head_addr = NULL;
		used_del_cb.call_back.usr_delete_call_back = NULL;
	}
	return true;
}

void *
(*get_match_call_back(void *reg_list_h))(void *node,void *compare_data)
{
	//还是遍历链表来获取回调函数
	if(NULL == match_call_back_list) {
		fprintf(stderr,"error happened in %s,because no call back regist",__func__);
		abort();
	}

	call_back_manage *temp = match_call_back_list;
	if(reg_list_h == used_match_cb.list_head_addr)
		return used_match_cb.call_back.usr_match_call_back;
	while(NULL != temp && temp->list_head_addr != reg_list_h)
		temp = temp->node;
	if(NULL == temp) {
		fprintf(stderr,"error happened in %s,because no call back found for this list",__func__);
		abort();
	}
	used_match_cb.list_head_addr = temp->list_head_addr;
	used_match_cb.call_back.usr_match_call_back = \
		temp->call_back.usr_match_call_back;
	return temp->call_back.usr_match_call_back;
}

bool
check_match_regist_state(void *reg_list_h)
{
	//检查链表，检查到就返回
	if(NULL == match_call_back_list)
		return false;//这种情况肯定没有任何函数注册
	call_back_manage *temp = match_call_back_list;//获取链表
	if(reg_list_h == used_match_cb.list_head_addr)
			return true;//先检测记录的缓存
	while(NULL != temp && temp->list_head_addr != reg_list_h)
		temp = temp->node;//检查到链表尾
	if(NULL == temp)
		return false;//说明实在没有
	return true;//说明有注册的情况
}

bool
regist_match_call_back(void *reg_list_h,
		void *(*usr_match_call_back)(void *,void *))
{
	//其他链表都在这里注册回调函数
	//不过在注册之前需要确保这个链表之前的
	//注册回调被卸载掉
	unregist_match_call_back(reg_list_h);
	call_back_manage *new = (call_back_manage *)malloc(sizeof(call_back_manage));
	new->node = NULL;
	new->list_head_addr = reg_list_h;
	new->call_back.usr_match_call_back = usr_match_call_back;
	//进行链表连接
	if(NULL == match_call_back_list)
		match_call_back_list  = new;
	else
		sl_insert_tail_relative(match_call_back_list,new);
	return true;
}

bool
unregist_match_call_back(void *reg_list_h)
{
	if(NULL == match_call_back_list)
		return false;

	call_back_manage *temp = match_call_back_list;

	//找到匹配的节点
	while(NULL != temp && \
		temp->list_head_addr != reg_list_h)
		temp = temp->node;

	if(NULL == temp)
		return false;
	match_call_back_list = delete_manage_call_back_del(match_call_back_list,temp);
	//防止出问题
	if(used_match_cb.list_head_addr == reg_list_h) {
		used_match_cb.list_head_addr = NULL;
		used_match_cb.call_back.usr_match_call_back = NULL;
	}
	return true;
}

