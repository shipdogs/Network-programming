#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <strings.h>
#include <fcntl.h>

#define MAXLINE 300
#define MAXNAME 20
#define SERV_PORT 8888
#define MAXFILE 1024
#define FINISHFLAG "|_|_|"
#define SERV_IP "192.168.183.128"

struct sockaddr_in saddr;
//   发送信息缓存        接送信息缓存               文件信息缓存
char buf[MAXLINE + 50], receivemsg[MAXLINE + 50], filebuf[MAXFILE + 50];
char msg[MAXLINE]; // 私聊信息
char chat[MAXLINE];
int sockfd, n,c=0;
char IP[INET_ADDRSTRLEN + 5];
char name[MAXNAME];		  // 用户名
char friendname[MAXNAME]; // 好友名
int stop = 0;
int flag = 0;
pthread_t tid;

void *listening();          // 监听接收信息线程函数
int upload_file();			// 上传文件
int download_file();		// 下载文件
void sendonemsg(char *msg); // 发送消息
void startlistening();		// 开启接收消息线程
void login(void);			// 登陆界面
//主函数
int main()
{
	// 输入IP，若为空则使用默认IP
	printf("输入服务器IP地址(default: 192.168.183.128):\n");
	fgets(IP, INET_ADDRSTRLEN + 5, stdin);
	IP[strlen(IP) - 1] = 0; // 过滤回车
	while (0 != strcmp(IP, SERV_IP))
	{
		if (strlen(IP) == 0)
		{
			strcpy(IP,SERV_IP);
			break;
		} // 空白输入，使用默认IP
		printf("错误IP地址,请重试:\n");
		fgets(IP, INET_ADDRSTRLEN + 5, stdin);
		IP[strlen(IP) - 1] = 0;
	}
	IP[strlen(IP)] = 0;

	// 连接服务器
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&saddr, sizeof(saddr));
	saddr.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &saddr.sin_addr);
	saddr.sin_port = htons(SERV_PORT);
	if (connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
	{
		perror("无法连接到服务器");
		return 0;
	}
	printf("已连接服务端!!!\n");
	sleep(1);
	// 登录界面
	login();
	printf("输入 ':q' 退出聊天室\n");
	printf("输入 ':r' 改名\n");
	printf("输入 ':s' 显示所有在线用户\n");
	printf("输入 ':f' 显示所有云端文件\n");
	printf("输入 ':u' 上传文件\n");
	printf("输入 ':d' 下载文件\n");
	printf("输入 ':a' 添加好友\n");
	printf("输入 ':k' 删除好友\n");
	printf("输入 ':p' 查看好友\n");
	printf("输入 ':c' 私聊(仅好友)\n");
	// 从这之后主线程只能发送消息，子线程只能接收消息
	startlistening();
	while (fgets(buf, MAXLINE, stdin) != NULL)
	{
		buf[strlen(buf) - 1] = 0; // 过滤结尾回车
		int quit = 0;
		if (buf[0] == ':')
		{ // 输入为命令语句
			if (buf[1] == 'q')
				quit = 1;			// 将退出标志置一
			else if (buf[1] == 'r') // 修改名字需要向服务器申请修改
			{
				bzero(name, MAXNAME);
				printf("请输入您的新名称: ");
				fflush(stdout);
				fgets(name, MAXNAME, stdin);
				name[strlen(name) - 1] = 0; // 过滤回车
				bzero(buf, MAXLINE + 50);
				sprintf(buf, ":r %s", name); // 拼接 :r newname
				sendonemsg(buf);
				continue;
			}
			else if (buf[1] == 's')
				; // 这两个功能只需发送命令代码
			else if (buf[1] == 'f')
				; // 接收线程将服务器发来的数据显示
			else if (buf[1] == 'u')
			{
				stop = 1;
				if (upload_file())
				{
					printf("上传成功\n");
				}
				while (stop)
					; // 上传文件时暂停主函数运行，直至完成上传
				bzero(buf, MAXLINE + 50);
				continue;
			}
			else if (buf[1] == 'd')
			{
				stop = 1;
				if (download_file())
				{
					printf("下载成功\n");
				}
				else
				{
					puts("服务端没有该文件");
				}
				while (stop)
					; // 下载文件时暂停主函数运行，直至完成下载
				memset(buf, 0, sizeof(buf));
				continue;
			}
			else if (buf[1] == 'a')
			{
				bzero(friendname, MAXNAME);
				printf("请输入添加好友的名称: ");
				fflush(stdout);
				fgets(friendname, MAXNAME, stdin);
				friendname[strlen(friendname) - 1] = 0; // 过滤回车
				bzero(buf, MAXLINE + 50);
				sprintf(buf, ":a %s", friendname); // 拼接 :a friendname
				sendonemsg(buf);
				continue;
			}
			else if (buf[1] == 'p') // 查看好友
				;
			else if (buf[1] == 'k') // 删除好友
			{
				printf("请输入要删除的好友的名称: ");
				fflush(stdout);
				char friendname[MAXNAME];
				fgets(friendname, MAXNAME, stdin);
				friendname[strlen(friendname) - 1] = 0; // 过滤回车
				bzero(buf, MAXLINE + 50);
				sprintf(buf, ":k %s", friendname); //: k friendname
				sendonemsg(buf);
				continue;
			}
			else if (buf[1] == 'c') // 好友私聊
			{
				printf("请输入私聊的好友的名称: ");
				fflush(stdout);
				char friendname[MAXNAME];
				pthread_cancel(tid); // 关闭线程让主线程来接收校验信息
				fgets(friendname, MAXNAME, stdin);
				friendname[strlen(friendname) - 1] = 0; // 过滤回车
				sprintf(buf, ":c %s", friendname);
				sendonemsg(buf); // :c friendname
				// 接收一下服务器反馈回来的信息
				recv(sockfd, buf, MAXLINE + 50, 0);
				startlistening(); // 接收完信息就可以开启线程监听了
				if (0 == strcmp(buf, "OK\n"))
				{
					// 可以私聊
					sprintf(chat, "私聊 %s>>: ", friendname);
					c=1;
					usleep(100000);
					sprintf(msg,"%s与你建立了私聊\n",name);
					// printf("发送了私聊建立\n");
					send(sockfd,msg,MAXLINE,0);//发送给服务端
					while (1)
					{
						// 读取用户输入
						bzero(msg, MAXLINE);
						fgets(msg, MAXLINE, stdin);
						// 处理用户输入
						send(sockfd, msg, MAXLINE, 0);
						if (0 == strcmp(msg, ":Q\n"))
						{
							// 退出私聊
							c=0;
							printf("已退出私聊\n");
							break; //
						}
					}
					continue;
				}
				else if (0 == strcmp(buf, "NOONLINE\n"))
				{
					printf("对方不在线\n");
				}
				else if (0 == strcmp(buf, "NOFRIEND\n"))
				{
					printf("对方不是您的好友\n");
				}
				continue;
			}
			else if (buf[1] == 't') // 测试用的，删除某张表
			{
				printf("请输入要删除的表名: ");
				fflush(stdout);
				char tablename[32];
				fgets(tablename, 32, stdin);
				tablename[strlen(tablename) - 1] = 0; // 过滤回车
				bzero(buf, MAXLINE + 50);
				sprintf(buf, ":t %s", tablename); //: t friend
				sendonemsg(buf);
				continue;
			}
			else
			{ // 非法命令
				printf("此命令不存在\n");
				bzero(buf, MAXLINE + 50);
				continue;
			}
		}
		sendonemsg(buf); // 发送命令或消息
		// :q
		bzero(buf, MAXLINE + 50);
		if (quit == 1)
		{
			bzero(buf, MAXLINE + 50);
			sprintf(buf, "我先退出咯,你们慢慢玩!\n");
			sendonemsg(buf);
			break;
		}
	}
	close(sockfd);
	return 0;
}
//登录界面
void login(void)
{
	// 实现登录功能
	char password[32];
	int ret;
	while (1)
	{
		system("clear");
		printf("**********************************\n");
		printf("************ 1. 注册 *************\n");
		printf("************ 2. 登录 *************\n");
		printf("************ 3. 退出 *************\n");
		printf("**********************************\n");
		int choice;
		char input[100];
		fgets(input, sizeof(input), stdin);
		if (sscanf(input, "%d", &choice) != 1)
		{
			printf("无效输入，请输入一个数字。\n");
			sleep(2);
			continue;
		}
		switch (choice)
		{
		case 1:
			// 注册功能
			printf("请输入您的用户名 >>");
			bzero(name, MAXNAME);
			scanf("%s", name);
			getchar();
			printf("请输入您的密码   >>");
			scanf("%s", password);
			getchar();
			// 拼接name和password  eg.:n cake 123
			sprintf(buf, ":n %s %s", name, password);
			send(sockfd, buf, MAXLINE + 50, 0);
			// 接收服务器发来的信息进行判断是否注册成功
			bzero(receivemsg, MAXLINE + 50);
			if (0 >= (ret = recv(sockfd, receivemsg, MAXLINE + 50, 0)))
			{
				perror("接收数据失败");
			}
			if (strcmp(receivemsg, "OK\n") == 0)
			{
				printf("注册成功!\n");
				sleep(2);
			}
			else
			{
				printf("该用户名已存在!\n");
				sleep(2);
			}
			break;
		case 2:
			// 登录功能
			printf("请输入您的用户名 >>");
			bzero(name, MAXNAME);
			scanf("%s", name);
			getchar();
			printf("请输入您的密码   >>");
			scanf("%s", password);
			getchar();
			// 拼接name和password  eg.:n cake 123
			sprintf(buf, ":l %s %s", name, password);
			send(sockfd, buf, MAXLINE + 50, 0);
			// 接收服务器发来的信息进行判断是否登录成功
			bzero(buf, MAXLINE + 50);
			recv(sockfd, buf, MAXLINE + 50, 0);
			if (strcmp(buf, "OK\n") == 0)
			{
				printf("登录成功!\n");
				sleep(1);
				return;
			}
			else if (strcmp(buf, "NO\n") == 0)
			{
				printf("登录错误,请重新登录!\n");
				sleep(2);
			}
			printf("buf: %s\n", buf);
			break;
		case 3:
			// 退出功能
			printf("感谢您的使用!\n");
			sleep(1);
			exit(0);
		default:
			printf("请重新选择!!!\n");
			sleep(2);
			break;
		}
	}
}
//开启接收线程
void startlistening(void)
{
	int ret = pthread_create(&tid, NULL, listening, NULL);
	if (ret != 0)
	{
		printf("创建线程失败\n");
		exit(0);
	}
	ret = pthread_detach(tid);
	if (ret != 0)
		exit(0);
}
//子线程---负责接收服务端的信息
void *listening(void *arg)
{
	while (1)
	{
		memset(receivemsg, 0, MAXLINE + 50);
		if(c){
			printf("%s",chat);
			fflush(stdout);
		}
		n = recv(sockfd, receivemsg, MAXLINE + 50, 0);
		if (n <= 0)
		{
			printf("服务器已关闭\n");
			close(sockfd);
			exit(0);
		}
		else if (0 == strcmp(receivemsg, ":Q\n"))
		{
			c=0;
			printf("您的好友已退出\n");
		}
		else
			printf("%s", receivemsg);
	}
}
// 向服务器发送消息(buf)
void sendonemsg(char *msg) { send(sockfd, msg, MAXLINE + 50, 0); }
// 上传文件 成功返回1，失败返回0
int upload_file(void)
{
	// 输入文件路径及文件名，以二进制读取方式打开文件
	printf("输入文件路径及文件名(for example ./file):\n");
	char filename[MAXLINE];
	fgets(filename, MAXLINE, stdin);	   //  ./OSI.png
	filename[strlen(filename) - 1] = '\0'; // 去除回车符
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		printf("本地没有此文件\n");
		stop = 0;
		return 0;
	}
	// 退出时需要重新启动主函数的运行
	// 计算文件字节数
	struct stat st;
	stat(filename, &st);
	int size = st.st_size;
	int total = 0;
	// 清除路径，只保留文件名，并转换为命令+参数格式发送给服务器
	int nn = strlen(filename) - 1;
	while (nn >= 0 && filename[nn] != '/')
		nn--;
	while (nn + 1)
	{
		strcpy(filename, filename + 1);
		nn--;
	}
	memset(buf, 0, MAXLINE + 50);
	sprintf(buf, ":u %s", filename);
	sendonemsg(buf);
	usleep(100000); // 等待服务器进行处理
	// 此处判断是否发送错误4或5，出现则不再发送
	if (stop == 0)
	{
		fclose(fp);
		return 0;
	}
	memset(filebuf, 0, sizeof(filebuf));
	pthread_cancel(tid); // 发送时关闭接收信息线程，防止发送被打断
	// 开始发送文件
	while (nn = fread(filebuf, sizeof(char), MAXFILE, fp))
	{
		if (nn == 0)
			break;										  // 发送结束
		total += nn;									  // 累加已发送的字节数
		printf("%6.2f%%", (float)total / (size) * 100.0); // 显示已发送的百分比
		send(sockfd, filebuf, nn, 0);					  // 读到多少个就发多少个
		printf("\b\b\b\b\b\b\b");
		bzero(filebuf, MAXFILE);
	}
	startlistening(); // 发送结束，重新开启接收信息线程
	strcpy(buf, FINISHFLAG);
	usleep(1000000); // 等待服务器处理完最后一个数据包后
	sendonemsg(buf); // 发送结束标志
	stop = 0;		 // 主函数继续运行
	fclose(fp);
	return 1;
}
// 下载文件 成功返回1，失败返回0
int download_file()
{
	pthread_cancel(tid); // 下载时关闭接收信息线程，此函数进行接收
	// 输入并发送下载命令
	printf("输入服务器上的文件名:\n");
	char filename[MAXLINE] = {0};
	fgets(filename, MAXLINE, stdin);
	filename[strlen(filename) - 1] = '\0'; // 去除回车符
	memset(buf, 0, sizeof(buf));
	sprintf(buf, ":d %s", filename);
	sendonemsg(buf); // 发送给服务端的信息:d filename
	usleep(10000);
	n = recv(sockfd, filebuf, MAXFILE, 0);
	int size = 0;
	// 如果接收到错误信息则退出，同时还需开启接收信息线程
	if (0 == strcmp(filebuf, "NO\n"))
	{
		stop = 0;
		startlistening();
		return 0;
	}
	else // 否则接收的就是文件内容
	{
		filebuf[strlen(filebuf) - 1] = 0;
		size = atoi(filebuf);
	}
	// printf("接收的文件大小为%d\n",size);
	// 以二进制形式写文件
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		printf("打开文件失败\n");
		stop = 0;
		startlistening();
		return 0;
	}
	int total = 0;
	// 下载中断后删除未下载完成的文件
	char Command[MAXLINE + 10];
	sprintf(Command, "rm -f %s", filename);
	while (1)
	{
		bzero(filebuf, MAXFILE);
		printf("%6.2f%%", (float)total / (size) * 100.0); // 输出已下载的百分比
		n = recv(sockfd, filebuf, MAXFILE, 0);
		printf("\b\b\b\b\b\b\b"); // 只显示一个进度百分比
		if (n <= 0)
		{ // 下载中断，删除文件并退出
			printf("服务器已关闭\n");
			fclose(fp);
			system(Command);
			exit(0);
		}
		if (strcmp(filebuf, FINISHFLAG) == 0)
		{ // 接收到结束标志，下载成功
			break;
		}
		fwrite(filebuf, n, 1, fp);
		fflush(fp);
		total += n; // 累加已发送字节数
	}
	startlistening(); // 开启接收信息线程
	stop = 0;		  // 主函数继续运行
	fclose(fp);
	return 1;
}
