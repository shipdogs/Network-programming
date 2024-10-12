#include "../inc/linklist.h"

// 创建链表
plist create_list()
{
    list *p = (list *)malloc(sizeof(list));
    if (NULL == p)
    {
        perror("Failed to open linked list space");
        return NULL;
    }
    p->pnext = NULL;
    return p;
}
// 尾插法
void insert_tail_list(list *p, type data, char *name)
{
    // 开辟一个空间存放插入的新节点
    list *node = (list *)malloc(sizeof(list));
    if (NULL == node)
    {
        perror("Failed to open linked list space");
        return;
    }
    // 给插入的节点赋值
    node->data = data;
    strcpy(node->name, name);
    node->pnext = NULL;
    // 遍历尾节点的位置
    list *tail = p;
    while (tail->pnext)
    {
        tail = tail->pnext;
    }
    tail->pnext = node;
}
// 遍历链表
void show_list(list *l)
{
    if (NULL == l)
    {
        puts("linked list is empty");
        return;
    }
    list *p = l;
    while (p->pnext)
    {
        p = p->pnext;
        printf("%d (%s) ", p->data, p->name);
    }
    puts("");
}
// 链表判空 0:空，1:非空
int empty_list(list *l)
{
    if (NULL == l->pnext)
        return 0;
    else
        return 1;
}
// 求链表长度
int length_list(list *l)
{
    if (!empty_list(l))
    {
        puts("linked list is empty");
        return -1;
    }
    list *p = l;
    int len = 0;
    while (p->pnext)
    {
        p = p->pnext;
        len++;
    }
    return len;
}
// 查找fd
list *select_list(list *l, type data)
{
    if (!empty_list(l))
    {
        puts("linked list is empty");
        return NULL;
    }
    list *p = l->pnext;
    while (p)
    {
        if (data == p->data)
        {
            return p;
        }
        p = p->pnext;
    }
    return NULL;
}
// 查找name
list *select_name_list(list *l, char *name)
{
    if (!empty_list(l))
    {
        puts("linked list is empty");
        return NULL;
    }
    list *p = l->pnext;
    while (p)
    {
        if (strcmp(p->name, name) == 0)
        {
            return p;
        }
        p = p->pnext;
    }
    return NULL;
}
// 修改
void revise_list(list *l, char *oldname, char *newname)
{
    list *p = l;
    while (p = select_name_list(p, oldname))
    {
        strcpy(p->name, newname);
    }
}
// 删除
void del_list(list *l, type data)
{
    if (!empty_list(l))
    {
        puts("linked list is empty");
        return;
    }
    list *p = l;
    while (p->pnext)
    {
        if (p->pnext->data == data)
        {
            list *node = p->pnext;
            p->pnext = node->pnext;
            free(node);
            break;
        }
        p = p->pnext;
    }
}
// 初始化
void init_list(list *l)
{
    if (!empty_list(l))
    {
        puts("linked list is empty");
        return;
    }
    while (l->pnext)
    {
        list *p = l->pnext;
        l->pnext = p->pnext;
        free(p);
    }
}
// 释放
void free_list(list **l)
{
    init_list(*l);
    free(*l);
    *l = NULL;
}
