/*===============================================
*   文件名称：sqlite_interface.h
*   创 建 者：   cake
*   创建日期：2024年09月25日
*   描    述：
================================================*/
#ifndef __SQLITE_INTERFACE_H
#define __SQLITE_INTERFACE_H

#define SQL_MAX_BYTE 256

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

// 创建数据库和表
sqlite3 *openDB(void);
//打开创建好友表
sqlite3 *openFriend_DB(void);
// 注册添加用户名,密码
int login_insqlite(sqlite3 *db, char *name, char *password);
// 添加好友  friend_in -> friend_out friend_in添加friend_out为好友
int add_friend(sqlite3 *db, char *friend_in, char *friend_out);
//查询friend表中某用户的所有好友装入friendlist里面
int select_friend(sqlite3 *db,char *name,char **friendlist);
//判断两个用户是否是好友关系
int is_friend(sqlite3 *db, char *friend_in, char *friend_out);
// 查询某表中是否存在某字段 重名返回1，不重名返回0
int is_same_name(sqlite3 *db, char *table, char *field,char *data);
// 删除好友
int delete_friend(sqlite3 *db, char *name,char *friendname);
// 校验用户名和密码
int compare(sqlite3 *db, char *name, char *password);
//修改好友名
int update_friend(sqlite3 *db,char *oldfriend,char *newfriend);
//修改指定数据
int update_info(sqlite3 *db, char *old, char *new);
//删除某表
int delete_table(sqlite3 *db, char *tablename);


#endif