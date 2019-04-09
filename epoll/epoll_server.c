/*************************************************************************
 > File Name: epoll_server.c
 > Author: zhengdongqi
 > Mail: 1821260963@qq.com
 > Created Time: Mon 08 Apr 2019 19:10:21 CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define IPADDR      "192.168.1.44"
#define PORT        8731
#define MAXSIZE     1024
#define LISTENQ     5
#define FDSIZE      1000
#define EPOLLEVENTS 100

/*函数声明*/
/*创建套接字并进行绑定*/
int socket_bind(int port);
/*IO多路复用epoll*/
void do_epoll(int listenfd);
/*事件处理函数*/
void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf);
/*处理接收到的连接*/
void handle_accpet(int epollfd, int listenfd);
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

int main() {
    int  listenfd;
    listenfd = socket_bind(PORT);
    listen(listenfd, LISTENQ);
    do_epoll(listenfd);
    return 0;
}
/*创建套接字并进行绑定*/
int socket_bind(int port) {
    int  listenfd;
    struct sockaddr_in socket_addr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        DBG("socket_bind->\033[31m套接字创建失败: %s\033[0m\n", strerror(errno));
        return -1;
    }
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //端口重用
    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        DBG("socket_bind->\033[31m设置端口重用失败: %s\033[0m\n", strerror(errno));
        close(listenfd);
        return -1;
    }
    if (bind(listenfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1) {
        DBG("socket_bind->\033[31m绑定失败: %s\033[0m\n", strerror(errno));
        return -1;
    }
    return listenfd;
}
/*IO多路复用epoll*/
void do_epoll(int listenfd) {
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int ret;
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    //创建一个描述符
    epollfd = epoll_create(FDSIZE);
    //添加监听描述符事件
    add_event(epollfd, listenfd, EPOLLIN);
    for ( ; ; ) {
        //获取已经准备好的描述符事件
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, listenfd, buf);
    }
    close(epollfd);
}
/*事件处理函数*/
void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf) {
    int i;
    int fd;
    //进行选好遍历
    for (i = 0; i < num; i++) {
        fd = events[i].data.fd;
        //根据描述符的类型和事件类型进行处理
        if ((fd == listenfd) && (events[i].events & EPOLLIN))
            handle_accpet(epollfd, listenfd);
        else if (events[i].events & EPOLLIN)
            do_read(epollfd, fd, buf);
        else if (events[i].events & EPOLLOUT)
            do_write(epollfd, fd, buf);
    }
}
/*处理接收到的连接*/
void handle_accpet(int epollfd, int listenfd) {
    struct sockaddr_in socket_addr;
    int len = sizeof(struct sockaddr_in), fd;
    fd = accept(listenfd, (struct sockaddr*)&socket_addr, (socklen_t *)&len);
    if (fd == -1) {
        DBG("handle_accpet->\033[31m接受套接字失败: %s\033[0m", strerror(errno));
    } else {
        DBG("handle_accpet->\033[33m检查到一个新用户: %s->%d\033[0m\n", inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port));
        
        //添加一个客户描述符和事件
        add_event(epollfd, fd, EPOLLIN);
    }
}
/*读处理*/
void do_read(int epollfd, int fd, char *buf) {
    int nread;
    nread = read(fd, buf, MAXSIZE);
    if (nread == -1) {
        DBG("do_read->\033[31m读❌\033[0m\n");
        fflush(stdout);
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    } else if (nread == 0) {
        DBG("\033[33m用户已下线\033[0m\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    } else {
        DBG("do_read->\033[33mread: %s\033[0m", buf);
        //修改描述符对应的事件，由读改为写
        modify_event(epollfd, fd, EPOLLOUT);
    }
}
/*写处理*/
void do_write(int epollfd, int fd, char *buf) {
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    if (nwrite == -1) {
        DBG("do_write->\033[31m❌\033[0m\n");
        fflush(stdout);
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    } else
        DBG("do_write->\033[33mwrite: %s\033[0m", buf);
    modify_event(epollfd, fd, EPOLLIN);
    memset(buf,0,MAXSIZE);
}
/*添加事件*/
void add_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd,&ev);
}
/*删除事件*/
void delete_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd,&ev);
}
/*修改事件*/
void modify_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd,&ev);
}
