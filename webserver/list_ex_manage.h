/*
 * list_ex.h
 *
 *  Created on: 2011-12-22
 *      Author: mobilefzb
 */

#ifndef LIST_EX_H_
#define LIST_EX_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define	STR(s)	#s
/**
 * @brief 单链表类型
 *
 * 在自己的数据结构中添加上这个
 * 类型（注意：这里将这个单链表
 * 类型作为自己的数据结构的第一
 * 个成员，一定要使用默认名字
 * node），自己的数据结构就可
 * 以使用这个单链表内建的一些函
 * 数进行链表操作了。
 */
typedef void sl_node;

/**
 * @brief 回调函数管理数据结构
 *
 * 该数据结构是回调管理的基本数据结构
 * 主要匹配了链表头和对应的回调函数方
 * 便使用
 */
typedef struct
{
	sl_node *node;
	void *list_head_addr;
	union cb {
		void (*usr_delete_call_back)(void *);
		void *(*usr_match_call_back)(void *,void *);
	} call_back;
} call_back_manage;

/**
 * @brief 获取删除回调函数
 *
 * 获取指定链表的删除回调函数
 */
extern void
(*get_delete_call_back(void *reg_list_h))(void *);

/**
 * @brief 检测删除回调管理链表是为指定链表注册的回调
 *
 * 主要是通过检测删除回调管理链表来判断这个链表是否注册了
 * 回调函数
 *
 * @param reg_list_h 要检测的链表头指针
 *
 * @return 返回true表示检测到回调，false表示没有
 */
extern bool
check_delete_regist_state(void *reg_list_h);

/**
 * @brief 注册删除回调函数
 *
 * 为一个链表注册一个节点数据释放的
 * 回调函数
 */
extern bool
regist_delete_call_back(void *reg_list_h,
			void (*usr_delete_call_back)(void *));

/**
 * @brief 卸载删除回调函数
 *
 * 解除一个链表和一个回调函数的绑定
 */
extern bool
unregist_delete_call_back(void *reg_list_h);

/**
 * @brief 获取匹配回调函数
 *
 * 获取指定链表的匹配函数
 */
extern void *
(*get_match_call_back(void *reg_list_h))(void *,void *);

/**
 * @brief 检测匹配回调管理链表是否为指定链表注册的回调
 *
 * 主要是检测匹配回调函数管理链表中是否有为指定链表注册的回调函数
 *
 * @param reg_list_h 要检测的链表头指针
 *
 * @return 返回true表示检测到，返回false表示没有检测到
 */
extern bool
check_match_regist_state(void *reg_list_h);

/**
 * @brief 注册匹配回调函数
 *
 * 为指定链表注册一个数据匹配的回调函数
 */
extern bool
regist_match_call_back(void *reg_list_h,
		void *(*usr_match_call_back)(void *,void *));

/**
 * @brief 卸载匹配回调函数
 *
 * 解除指定链表和一个匹配回调函数的绑定
 */
extern bool
unregist_match_call_back(void *reg_list_h);

#endif /* LIST_EX_H_ */
