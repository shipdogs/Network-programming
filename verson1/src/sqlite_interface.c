/*===============================================
*   文件名称：sqlite_interface.c
*   创 建 者：   cake
*   创建日期：2024年09月25日
*   描    述：
================================================*/
#include "../inc/sqlite_interface.h"

// 打开创建用户密码表
sqlite3 *openDB(void)
{
    sqlite3 *db = NULL;
    char sql[SQL_MAX_BYTE];
    char *err;
    // 打开一个数据库
    if (SQLITE_OK != sqlite3_open("WX.db", &db))
    {
        fprintf(stderr, "open:%s\n", sqlite3_errmsg(db));
    }
    // 创建表格
    bzero(sql, SQL_MAX_BYTE);
    strcpy(sql, "create table if not exists user(name char(20) primary key,password char(32));");
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "create table error: %s\n", err);
    }
    return db;
}
// 打开创建好友表
sqlite3 *openFriend_DB(void)
{
    sqlite3 *db = (sqlite3 *)malloc(1024);
    char sql[SQL_MAX_BYTE];
    char *err;
    // 打开一个数据库
    if (SQLITE_OK != sqlite3_open("WX.db", &db))
    {
        fprintf(stderr, "open:%s\n", sqlite3_errmsg(db));
    }
    // 创建表格
    bzero(sql, SQL_MAX_BYTE);
    strcpy(sql, "create table if not exists friend(friend_in char(20) ,friend_out char(20));");
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "create table error: %s\n", err);
    }
    return db;
}
// 注册添加用户名,密码
int login_insqlite(sqlite3 *db, char *name, char *password)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "insert into user(name,password) values('%s','%s');", name, password);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "insert error: %s\n", err);
        return 0;
    }
    return 1;
}
// 添加好友  friend_in -> friend_out friend_in添加friend_out为好友
int add_friend(sqlite3 *db, char *friend_in, char *friend_out)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "insert into friend(friend_in,friend_out) values('%s','%s');", friend_in, friend_out);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "add_friend error: %s\n", err);
        return 0;
    }
    return 1;
}
// 判断两个用户是否是好友关系
int is_friend(sqlite3 *db, char *friend_in, char *friend_out)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    char **result;
    int nrow, ncol;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "select * from friend where (friend_in = '%s' and friend_out = '%s') or (friend_in = '%s' and friend_out = '%s');", friend_in, friend_out, friend_out, friend_in);
    if (SQLITE_OK != sqlite3_get_table(db, sql, &result, &nrow, &ncol, &err))
    {
        fprintf(stderr, "is_friend error: %s\n", err);
        return 0;
    }
    return nrow > 0 ? 1 : 0;
}
// 查询friend表中某用户的好友
int select_friend(sqlite3 *db, char *name, char **friendlist)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    char **result;
    int nrow, ncol;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "select * from friend where friend_in = '%s' or friend_out = '%s';", name, name);
    if (SQLITE_OK != sqlite3_get_table(db, sql, &result, &nrow, &ncol, &err))
    {
        fprintf(stderr, "select_friend error: %s\n", err);
        return 0;
    }
    int i = 0;
    for (i = 1; i <= nrow; i++)
    {
        if (0 == strcmp(result[i * ncol + 0], name))
        {
            // printf("%s\n", result[i * ncol + 1]);
            strcpy(friendlist[i - 1], result[i * ncol + 1]);
        }
        else if (0 == strcmp(result[i * ncol + 1], name))
        {
            // printf("%s\n", result[i * ncol + 0]);
            strcpy(friendlist[i - 1], result[i * ncol + 0]);
        }
    }
    for (int j = i - 1; j < 50; j++)
    {
        friendlist[j] = NULL;
    }

    return 1;
}
// 查询某表中是否存在某字段 重名返回1，不重名返回0
int is_same_name(sqlite3 *db, char *table, char *field, char *data)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    char **result;
    int nrow, ncol;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "select %s from %s;", field, table);
    if (SQLITE_OK != sqlite3_get_table(db, sql, &result, &nrow, &ncol, &err))
    {
        fprintf(stderr, "is_same_name error: %s\n", err);
    }
    for (int i = 1; i <= nrow; i++)
    {
        for (int j = 0; j < ncol; j++)
        {
            if (0 == strcmp(result[i * ncol + j], data))
            {
                return 1;
            }
        }
    }
    return 0;
}
// 删除好友
int delete_friend(sqlite3 *db, char *name, char *friendname)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "delete from friend where (friend_in = '%s' and friend_out = '%s') or (friend_in = '%s' and friend_out = '%s');", name, friendname, friendname, name);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "delete_friend error: %s\n", err);
        return 0;
    }
    return 1;
}
// 校验用户名和密码 成功返回1，失败返回0
int compare(sqlite3 *db, char *name, char *password)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    char **result;
    int nrow, ncol;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "select name,password from user;");
    if (SQLITE_OK != sqlite3_get_table(db, sql, &result, &nrow, &ncol, &err))
    {
        fprintf(stderr, "select error: %s\n", err);
    }
    for (int i = 1; i <= nrow; i++)
    {
        for (int j = 0; j < ncol; j++)
        {
            if (0 == strcmp(result[i * ncol + j], name))
            {
                if (0 == strcmp(result[i * ncol + j + 1], password))
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}
// 修改好友名
int update_friend(sqlite3 *db, char *oldfriend, char *newfriend)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "update friend set friend_out='%s' where friend_out='%s';", newfriend, oldfriend);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "update_friend error: %s\n", err);
        return 0;
    }
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "update friend set friend_in='%s' where friend_in='%s';", newfriend, oldfriend);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "update_friend error: %s\n", err);
        return 0;
    }
    return 1;
}
// 修改指定数据
int update_info(sqlite3 *db, char *old, char *new)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "update user set name='%s' where name='%s';", new, old);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "update error: %s\n", err);
        return 0;
    }
    return 1;
}
// 删除某表
int delete_table(sqlite3 *db, char *tablename)
{
    char sql[SQL_MAX_BYTE];
    char *err;
    bzero(sql, SQL_MAX_BYTE);
    sprintf(sql, "drop table %s;", tablename);
    if (SQLITE_OK != sqlite3_exec(db, sql, NULL, NULL, &err))
    {
        fprintf(stderr, "delete table error: %s\n", err);
        return 0;
    }
    return 1;
}
