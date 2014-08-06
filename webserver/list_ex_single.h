/*
 * list_ex_single.h
 *
 *  Created on: 2011-12-21
 *      Author: mobilefzb
 */

#ifndef LIST_EX_SINGLE_H_
#define LIST_EX_SINGLE_H_
#include <stdint.h>//变量规范化
#include <stdbool.h>//逻辑变量标准化
#include <list_ex_manage.h>

/**
 * @brief 单链表尾部插入（整个链表后）
 *
 * 将节点插到链表最后一个节点的结尾处
 */
#define	sl_node_insert_tail_absolute(head,new)	\
	({ \
		typeof(head) temp = head; \
		while(NULL != (temp)->node)	\
			(temp) = (temp)->node;	\
		(temp)->node = new;	\
	})

/**
 * @brief 单链表相对尾部插入
 *
 * 将节点插入到链表任意节点之后
 * 这里需要注意的是不会检测被插
 * 入的节点是否在想要插入的链表
 * 内存在，这个应该由用户自己来
 * 进行判断
 */
#define	sl_insert_tail_relative(head,new) \
	({ \
		(new)->node = (head)->node; \
		(head)->node = new; \
	})
/**
 * @brief 单链表头部快速插入（链表的头前面）
 *
 * 将节点插入到链表的指定链表头部的前面
 * 注意，这个方法是不安全的。如果你已经
 * 为这个链表注册了函数那么使用这个函数
 * 必然导致严重的错误，当不清楚自己是否
 * 注册了回调的时候最好不使用这个函数
 */
#define	sl_insert_head_absolute_fast(head,new) \
	({ \
		(new)->node = head; \
		head = new; \
	})

/**
 * @brief 单链表头部插入（链表的头前面）
 *
 * 将节点插入到链表的指定链表头部的前面
 */
#define	sl_insert_head_absolute(head,new) \
	({ \
		void (*usr_delete_call_back)(void *) = NULL; \
		void *(*usr_match_call_back)(void *,void *) = NULL; \
		if(check_delete_regist_state(head)) { \
			usr_delete_call_back = get_delete_call_back(head); \
			unregist_delete_call_back(head); \
		} \
		if(check_match_regist_state(head)) { \
			usr_match_call_back = get_match_call_back(head); \
			unregist_match_call_back(head); \
		} \
		(new)->node = head; \
		head = new; \
		if(NULL != usr_delete_call_back) \
			regist_delete_call_back(head,usr_delete_call_back); \
		if(NULL != usr_match_call_back) \
			regist_match_call_back(head,usr_match_call_back); \
	})

/**
 * @brief 单链表相对头部插入
 *
 * 将节点插入到任意节点之前如果链表中
 * 不存在current的节点则该操作等于无
 * 效所以current是否在head为头的链表
 * 中需要用户进行自行检测
 */
#define	sl_insert_head_relative(head,current,new) \
	({ \
		if(head == current) \
			sl_insert_head_absolute(head,new); \
		else { \
			typeof(head) temp = head; \
			while(NULL != (temp)->node && \
					current != (temp)->node) \
				temp = (temp)->node; \
			if((temp)->node == current) { \
				(temp)->node = new; \
				(new)->node = current; \
			} \
		} \
	})

/**
 * @brief 单链表节点删除
 *
 * 将指定的节点进行删除，注意自己
 * 使用这个函数的时候要注意注册一个
 * 用于对应数据结构节点内存释放的函数
 */
#define	sl_delete(head,del_node) \
	({ \
		if(head == del_node) { \
			void (*usr_delete_call_back)(void *) = NULL; \
			void *(*usr_match_call_back)(void *,void *) = NULL; \
			usr_delete_call_back = get_delete_call_back(head); \
			unregist_delete_call_back(head); \
			if(check_match_regist_state(head)) { \
				usr_match_call_back = get_match_call_back(head); \
				unregist_match_call_back(head); \
			} \
			typeof(head) sl_head = head; \
			head = (head)->node; \
			usr_delete_call_back(sl_head); \
			if(NULL != head) { \
				regist_delete_call_back(head, \
					usr_delete_call_back); \
				if(NULL != usr_match_call_back) \
					regist_match_call_back(head, \
						usr_match_call_back); \
			} \
		} else { \
			typeof(head) sl_head = head; \
			while((sl_head)->node != NULL \
				&& (sl_head)->node != del_node) \
				sl_head = (sl_head)->node; \
			if((sl_head)->node == del_node) { \
				(sl_head)->node = (del_node)->node; \
				(get_delete_call_back(head))(del_node); \
			} \
		} \
	})

/**
 * @brief 查找链表数据匹配的节点
 *
 * 查找匹配函数，这里主要是进行数据匹配
 * 需要根据用户数据进行匹配，所以需要回
 * 调函数最后返回匹配的节点
 */

#define	sl_match_node_data(head,found_node,data) \
	({ \
		found_node = (get_match_call_back(head))(head,data); \
	})

/**
 * @brief 链表销毁
 *
 * 将一个链表进行销毁，所有节点数据
 * 全部进行消除，内存进行释放
 */
#define	sl_destroy(head) \
	({ \
		typeof(head) tmp = head; \
		tmp = head->node; \
		while(NULL != head) { \
			if(NULL != tmp) { \
				sl_delete(head,tmp); \
				tmp = head->node; \
			} else \
				sl_delete(head,head); \
		} \
	})

#endif /* LIST_EX_SINGLE_H_ */
