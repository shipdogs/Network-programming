/*===============================================
*   文件名称：main.c
*   创 建 者：   cake
*   创建日期：2024年09月23日
*   描    述：测试函数
================================================*/
#include "../inc/ser_init.h"
#include "../inc/pthread_tasks.h"
#include "../inc/linklist.h"
#include "../inc/sqlite_interface.h"
int main(int argc, char *argv[])
{
    //初始化监听套接字
    int listenfd = tcp_server_init("0", SERV_PORT, MAXCON);
    if (-1 == listenfd)
    {
        perror("init");
        return -1;
    }
    //使用多线程实现并发服务器(死循环)
    business(listenfd);
    close(listenfd);
    return 0;

}
