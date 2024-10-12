#include "../inc/ser_init.h"

struct sockaddr_in caddr,saddr;
// 初始化监听套接字
int tcp_server_init(const char *ip, int port, int backlog)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
        return -1;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);
    // IP和PORT快速重用
    int ok = 1;
    if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof ok))
        return -1;
    if (-1 == bind(fd, (void *)&saddr, sizeof saddr))
        return -1;
    if (-1 == listen(fd, backlog))
        return -1;
    puts("服务端已开启,持续监听中...");
    return fd;
}
// 等待连接
int tcp_server_wait(int listenfd)
{
    socklen_t len = sizeof(caddr);
    int nfd = accept(listenfd, (void *)&caddr, &len);
    if (-1 == nfd)
        return -1;
    printf("客户端 %s:%d 已连接!\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
    return nfd;
}
