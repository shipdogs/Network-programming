#ifndef __LINKLIST_H
#define __LINKLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int type;

typedef struct node{
    type data;
    char name[20];
    struct node *pnext;
}list,*plist;
//约定头节点不存数据

//创建链表
plist create_list();
//链表判空 0:空，1:非空
int empty_list(list *p);
//求链表长度
int length_list(list *p);
//尾插法
void insert_tail_list(list *p,type data,char *name);
//遍历链表
void show_list(list *p);
//查找
list *select_list(list *p,type data);
//查找name
list *select_name_list(list *l,char *name);
//修改
void revise_list(list *p,char *oldname,char *newname);
//删除
void del_list(list *p,type data);
//初始化
void init_list(list *p);
//释放
void free_list(list **p);

#endif

