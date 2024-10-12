#ifndef __SER_H__
#define __SER_H__

#include "init.h"

// 初始化
int tcp_server_init(const char *ip, int port, int backlog);
// 等待连接
int tcp_server_wait(int listenfd);

#endif