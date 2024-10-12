/*===============================================
*   文件名称：pthread_tasks.h
*   创 建 者：   cake
*   创建日期：2024年09月25日
*   描    述：
================================================*/
#ifndef __PTHREAD_TASKS_H
#define __PTHREAD_TASKS_H

#include "init.h"
#include "linklist.h"
#include "sqlite_interface.h"

typedef struct
{
    int sockfd;
    plist head;
} DATA;

// 发送消息给所有在线客户端(除自己)
void send_msg_all(char *msg, plist p, int myfd);
//发送消息给指定用户/服务器
void send_msg_one(char *msg,int fd);
// 死循环处理服务端业务(出错返回-1)
void business(int listenfd);
// 子线程处理函数---处理客户端
void *handle_client(void *arg);
#endif