/*************************************************************************
	> File Name: epoll.h
	> Author: zhengdongqi
	> Mail: 1821260963@qq.com
	> Created Time: 一  4/ 1 13:14:26 2019
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAXSIZE     1024
#define FDSIZE      1000
#define EPOLLEVENTS 100

/*socket绑定*/
int socket_bind(int port);

/*IO多路复用epoll*/
void do_epoll(int listenfd);

/*事件处理函数*/
void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf);

/*处理接收到的连接*/
void handle_accpet(int epollfd, int listenfd);

/*处理进行连接*/
void handle_connect(int fd);

/*读处理*/
void do_read(int epollfd, int fd, char *buf);

/*写处理*/
void do_write(int epollfd, int fd, char *buf);

/*添加事件*/
void add_event(int epollfd, int fd, int state);

/*修改事件*/
void modify_event(int epollfd, int fd, int state);

/*删除事件*/
void delete_event(int epollfd, int fd, int state);

#endif
