/*===============================================
*   文件名称：init.h
*   创 建 者：   cake
*   创建日期：2024年09月25日
*   描    述：
================================================*/
#ifndef __INIT_H
#define __INIT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <dirent.h>
#define MAXLINE 300
#define MAXNAME 20
#define MAXPASSWORD 32
#define SERV_PORT 8888
#define MAXCON (10 + 1) // 最大连接数为10，最后一个用于临时连接
#define MAXFILE 1024
#define MAXFRIEND 50 //最多好友数
#define FINISHFLAG "|_|_|"

#endif