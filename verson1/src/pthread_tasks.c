/*===============================================
*   文件名称：pthread_tasks.c
*   创 建 者：   cake
*   创建日期：2024年09月25日
*   描    述：使用多线程实现并发服务器
================================================*/
#include "../inc/pthread_tasks.h"
#include "../inc/ser_init.h"
#include "../inc/linklist.h"

pthread_mutex_t mutex;

// 群发(除自己)
void send_msg_all(char *msg, plist p, int myfd)
{
    list *p1 = p->pnext;
    while (p1 != NULL)
    {
        if (p1->data != myfd)
            send(p1->data, msg, MAXLINE + 50, 0);
        p1 = p1->pnext;
    }
}
// 发送消息给指定用户
void send_msg_one(char *msg, int fd)
{
    send(fd, msg, MAXLINE + 50, 0);
}
// 子线程处理函数---处理客户端
void *handle_client(void *arg)
{
    int nfd = ((DATA *)arg)->sockfd; // 通信套接字
    plist p = ((DATA *)arg)->head;   // 链表头部
    extern struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    len = sizeof caddr;
    // 定义局部变量
    char value[MAXLINE];         // 保存命令参数
    char buf[MAXLINE + 50];      // 广播信息缓存区
    char spemsg[MAXLINE + 50];   // 指定用户信息缓存区
    char filebuf[MAXFILE];       // 传输文件缓存区
    char name[MAXNAME];          // 存放用户名
    char friendname[MAXNAME];    // 存放好友名
    char password[MAXPASSWORD];  // 存放密码
    char str[INET_ADDRSTRLEN];   // 存放IP地址
    char *friendlist[MAXFRIEND]; // 存放好友名
    while (1)
    {
        // 为每个指针分配内存
        for (int i = 0; i < 50; i++)
        {
            friendlist[i] = malloc(20 * sizeof(char));
            if (friendlist[i] == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }
        bzero(buf, MAXLINE + 50);
        // 接收客户端的信息
        if (0 < recv(nfd, buf, MAXLINE + 50, 0))
        {
            if (buf[0] == ':')
            {
                // 可能是命令继续判断下个字符
                if (buf[1] == 'n') // 注册
                {
                    bzero(value, MAXLINE);
                    strcpy(value, buf + 3); // 保存 用户名 密码
                    strcpy(name, strtok(value, " "));
                    strcpy(password, strtok(NULL, " "));
                    // 注册，判断是否重名(查询数据库看是否由这个名字)
                    bzero(buf, MAXLINE + 50);
                    // 打开数据库
                    pthread_mutex_lock(&mutex);
                    sqlite3 *db = openDB();
                    if (login_insqlite(db, name, password))
                    {
                        pthread_mutex_unlock(&mutex);
                        sprintf(buf, "OK\n");
                        send_msg_one(buf, nfd); // 只回复该客户端
                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex);
                        sprintf(buf, "NO\n");
                        send_msg_one(buf, nfd); // 只回复该客户端
                    }
                    sqlite3_close(db);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'l') // 登录
                {
                    bzero(value, MAXLINE);
                    strcpy(value, buf + 3); // 保存 用户名 密码
                    strcpy(name, strtok(value, " "));
                    strcpy(password, strtok(NULL, " "));
                    bzero(buf, MAXLINE + 50);
                    // 校验用户名和密码
                    pthread_mutex_lock(&mutex);
                    sqlite3 *db = openDB();
                    if (!select_name_list(p, name) && compare(db, name, password))
                    {
                        pthread_mutex_unlock(&mutex);
                        sprintf(buf, "OK\n");
                        send_msg_one(buf, nfd); // 只回复该客户端
                        bzero(buf, MAXLINE + 50);
                        sprintf(buf, "%s(%s:%d)进入了聊天室\n", name, inet_ntop(AF_INET, &caddr.sin_addr, str, sizeof(str)),
                                ntohs(caddr.sin_port));
                        send_msg_all(buf, p, nfd); // 给所有在线客户端发消息
                        // 加入在线客户端链表
                        pthread_mutex_lock(&mutex);
                        insert_tail_list(p, nfd, name);
                        pthread_mutex_unlock(&mutex);
                        bzero(buf, MAXLINE + 50);
                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex);
                        sprintf(buf, "NO\n");
                        send_msg_one(buf, nfd); // 只回复该客户端
                    }
                    sqlite3_close(db);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'r') // 改名
                {
                    char tempname[MAXNAME];
                    strcpy(tempname, name); // 先将原来名字拷贝一份
                    bzero(name, MAXNAME);
                    strcpy(name, buf + 3); // 保存 新用户名
                    // 改名，判断是否重名(查询数据库看是否有这个名字)
                    pthread_mutex_lock(&mutex);
                    sqlite3 *db = openDB();
                    if (update_info(db, tempname, name)) // 修改失败
                    {
                        sprintf(buf, "修改成功!\n");
                        send_msg_one(buf, nfd); // 只回复该客户端
                        // 改名成功，修改在线客户端链表中的名字
                        revise_list(p, tempname, name);
                        // 群发其他人
                        sprintf(buf, "%s修改新名字为%s\n", tempname, name);
                        send_msg_all(buf, p, nfd);
                        sqlite3 *fridb = openFriend_DB();
                        if (update_friend(fridb, tempname, name))
                            printf("修改好友名成功\n");
                        else
                            printf("修改好友名失败\n");
                        pthread_mutex_unlock(&mutex);
                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex);
                        sprintf(buf, "修改失败，恢复原来的名字%s\n", tempname);
                        send_msg_one(buf, nfd); // 只回复该客户端
                        // 改名失败，恢复原来的名字
                        strcpy(name, tempname);
                    }
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 's') // 显示在线用户
                {
                    bzero(buf, MAXLINE + 50);
                    char temp[50] = {0};
                    sprintf(buf, "在线用户:\n IP               Port    name\n");
                    pthread_mutex_lock(&mutex);
                    list *p1 = p->pnext;
                    struct sockaddr_in clientaddr;
                    socklen_t addrlen = sizeof(clientaddr);
                    while (p1)
                    {
                        getpeername(p1->data, (struct sockaddr *)&clientaddr, &addrlen);
                        sprintf(temp, "%-15s  %-5d    %-5s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), p1->name);
                        strcat(buf, temp);
                        p1 = p1->pnext;
                    }
                    pthread_mutex_unlock(&mutex);
                    send_msg_one(buf, nfd);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'f') // 显示云端文件
                {
                    bzero(buf, MAXLINE + 50);
                    char *dir = "./Files";
                    DIR *dp;
                    struct dirent *dirp;
                    if ((dp = opendir(dir)) == NULL)
                    {
                        perror("opendir");
                        continue;
                    }
                    while ((dirp = readdir(dp)) != NULL)
                    {
                        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
                        {
                            sprintf(buf, "%s\n", dirp->d_name);
                            send_msg_one(buf, nfd);
                        }
                    }
                    closedir(dp);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'u') // 上传文件
                {
                    char filename[MAXLINE] = {0};
                    sprintf(filename, "./Files/%s", buf + 3);
                    FILE *fp = fopen(filename, "wb");
                    if (fp == NULL)
                    {
                        printf("open file error\n");
                        exit(1);
                    }
                    bzero(filebuf, MAXFILE);
                    int len;
                    while ((len = recv(nfd, filebuf, MAXFILE, 0)) > 0)
                    {
                        if (0 == strcmp(filebuf, FINISHFLAG))
                        {
                            printf("接收完成!\n");
                            break;
                        }
                        fwrite(filebuf, 1, len, fp);
                        bzero(filebuf, MAXFILE);
                    }
                    fclose(fp);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'd') // 下载文件
                {
                    char filename[MAXLINE] = {0};
                    sprintf(filename, "./Files/%s", buf + 3);
                    printf("filename=%s\n", filename);
                    // 计算文件字节数
                    struct stat st;
                    stat(filename, &st);
                    int size = st.st_size;
                    FILE *fp = fopen(filename, "rb");
                    bzero(filebuf, MAXFILE);
                    if (fp == NULL)
                    {
                        sprintf(filebuf, "NO\n");
                        send(nfd, filebuf, MAXFILE, 0);
                        continue;
                    }
                    else // 打开成功发送文件的大小
                    {
                        sprintf(filebuf, "%d\n", size);
                        send(nfd, filebuf, MAXFILE, 0);
                    }
                    usleep(10000); // 确保客户端已接收到数据
                    int len;
                    while ((len = fread(filebuf, 1, MAXFILE, fp)) > 0)
                    {
                        send(nfd, filebuf, len, 0);
                        bzero(filebuf, MAXFILE);
                    }
                    usleep(1000000); // 确保客户端已接收到数据
                    strcpy(filebuf, FINISHFLAG);
                    send(nfd, filebuf, MAXFILE, 0);
                    fclose(fp);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'a') // 添加好友
                {
                    bzero(friendname, MAXNAME);
                    strcpy(friendname, buf + 3);
                    pthread_mutex_lock(&mutex);
                    sqlite3 *fridb = openFriend_DB();
                    sqlite3 *db = openDB();
                    bzero(buf, MAXLINE + 50);
                    // 如果好友存在与用户密码表中，并且他们不是好友关系
                    if (is_same_name(db, "user", "name", friendname) && !is_friend(fridb, name, friendname))
                    {
                        // 添加好友
                        if (add_friend(fridb, name, friendname))
                        {
                            sprintf(buf, "%s添加%s成功!\n", name, friendname);
                            send_msg_one(buf, nfd); // 回复该客户端
                        }
                        else
                        {
                            sprintf(buf, "服务端繁忙!!!\n");
                            send_msg_one(buf, nfd); // 回复该客户端
                        }
                    }
                    else
                    {
                        sprintf(buf, "你们已经是好友或者好友不存在!\n");
                        send_msg_one(buf, nfd); // 回复该客户端
                    }
                    pthread_mutex_unlock(&mutex);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'p') // 查看好友
                {
                    pthread_mutex_lock(&mutex);
                    sqlite3 *fridb = openFriend_DB();
                    if (select_friend(fridb, name, friendlist))
                    {
                        pthread_mutex_unlock(&mutex);
                        sprintf(buf, "好友列表:\n");
                        send_msg_one(buf, nfd);
                        for (int i = 0; friendlist[i] != NULL; i++)
                        {
                            bzero(buf, MAXLINE + 50);
                            sprintf(buf, "%s\t", friendlist[i]);
                            send_msg_one(buf, nfd);
                            free(friendlist[i]); // 释放内存
                        }
                        send_msg_one("\n", nfd);
                    }
                    else
                        printf("查询失败\n");
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'k') // 删除好友
                {
                    bzero(friendname, MAXNAME);
                    strcpy(friendname, buf + 3);
                    pthread_mutex_lock(&mutex);
                    sqlite3 *fridb = openFriend_DB();
                    // 先判断双方是否是好友
                    if (is_friend(fridb, name, friendname))
                    {
                        // 是好友再删除好友
                        if (delete_friend(fridb, name, friendname))
                        {
                            sprintf(buf, "%s删除%s成功!\n", name, friendname);
                            send_msg_one(buf, nfd);
                        }
                        else
                        {
                            sprintf(buf, "服务端繁忙!!!\n");
                            send_msg_one(buf, nfd);
                        }
                    }
                    else
                    {
                        sprintf(buf, "%s并未添加%s为好友!\n", name, friendname);
                        send_msg_one(buf, nfd);
                    }
                    pthread_mutex_unlock(&mutex);
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 'c') // 好友私聊
                {
                    char friendname[MAXNAME];
                    strcpy(friendname, buf + 3);
                    pthread_mutex_lock(&mutex);
                    sqlite3 *fridb = openFriend_DB();
                    // 判断friend表中是否有该好友
                    if (is_friend(fridb, name, friendname))
                    {
                        // 判断对方是否在线
                        if (select_name_list(p, friendname))
                        {
                            pthread_mutex_unlock(&mutex);
                            // 在线，发送OK\n信息给客户端
                            bzero(buf, MAXLINE + 50);
                            sprintf(buf, "OK\n");
                            send_msg_one(buf, nfd);
                            // 建立联系后可以转发给好友了
                            char temp[MAXLINE];
                            while (1)
                            {
                                bzero(buf, MAXLINE + 50);
                                bzero(temp, MAXLINE);
                                int ret;
                                if ((ret = recv(nfd, temp, MAXLINE, 0)) <= 0)
                                {
                                    sprintf(buf, "好友%s已断开\n", name);
                                    send_msg_one(buf, select_name_list(p, friendname)->data);
                                    break;
                                }
                                if (0 == strcmp(temp, ":Q\n"))
                                {
                                    sprintf(buf, "好友%s已断开\n", name);
                                    send_msg_one(buf, select_name_list(p, friendname)->data);
                                    break;
                                }
                                sprintf(buf, "\n来自%s的私聊消息>>%s", name, temp);
                                send_msg_one(buf, select_name_list(p, friendname)->data);
                            }
                        }
                        else
                        {
                            pthread_mutex_unlock(&mutex);
                            // 不在线
                            bzero(buf, MAXLINE + 50);
                            sprintf(buf, "NOONLINE\n");
                            send_msg_one(buf, nfd);
                        }
                    }
                    else
                    {
                        pthread_mutex_unlock(&mutex);
                        // 不是好友
                        bzero(buf, MAXLINE + 50);
                        sprintf(buf, "NOFRIEND\n");
                        send_msg_one(buf, nfd);
                    }
                    bzero(buf, MAXLINE + 50);
                }
                if (buf[1] == 't') // 测试---删除某表
                {
                    char tablename[MAXLINE + 50] = {0};
                    strcpy(tablename, buf + 3);
                    pthread_mutex_lock(&mutex);
                    sqlite3 *fridb = openFriend_DB();
                    if (delete_table(fridb, tablename))
                        printf("删除成功!\n");
                    else
                        printf("删除失败\n");
                    pthread_mutex_unlock(&mutex);
                    bzero(buf, MAXLINE + 50);
                }
            }
            else
            {
                // 非命令，作为消息发送给所有用户，并给消息来源发送提示
                time_t tnow = time(NULL);
                struct tm *tmnow = localtime(&tnow);
                char temp[MAXLINE];
                strcpy(temp, buf); // 把信息先复制一份存在temp中
                sprintf(buf, "\n\t%02u:%02u:%02u\n群聊消息>> %s: %s\n", tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec, name, temp);
                // 发送信息给所有在线客户端
                send_msg_all(buf, p, nfd);
            }
        }
        else
        {
            pthread_mutex_lock(&mutex);
            del_list(p, nfd); // 从在线客户端链表中删除
            pthread_mutex_unlock(&mutex);
            printf("用户%s已断开\n", name);
            bzero(buf, MAXLINE);
            if ((*name) == 0)
                strcpy(name, "某个神秘人");
            sprintf(buf, "%s潇洒的离开了\n", name);
            send_msg_all(buf, p, nfd); // 给所有在线客户端发消息
            close(nfd);                // 关闭与客户端的连接
            break;                     // 跳出循环，关闭线程并结束本次循环
        }
    }
}
//服务端主业务实现
void business(int listenfd)
{
    // 创建链表维护所有在线客户端
    plist head = create_list();
    // 初始化锁
    pthread_mutex_init(&mutex, NULL);
    pthread_t tid;
    // 死循环处理服务端业务
    while (1)
    {
        // TODO: 编写处理逻辑
        //  堵塞等待客户端连接
        int newfd = tcp_server_wait(listenfd);
        if (-1 == newfd)
        {
            perror("wait");
            continue; // 跳过本次循环，继续下一个循环
        }
        // 创建线程处理新连接
        DATA data = {newfd, head};
        pthread_create(&tid, NULL, handle_client, (void *)&data);
        pthread_detach(tid);
    }
}
