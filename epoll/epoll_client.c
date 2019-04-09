/*************************************************************************
 > File Name: epoll_client.c
 > Author: zhengdongqi
 > Mail: 1821260963@qq.com
 > Created Time: Mon 08 Apr 2019 19:15:59 CST
 ************************************************************************/
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define MAXSIZE     1024
#define IPADDR       "192.168.1.44"
#define PORT           8731
#define FDSIZE      1024
#define EPOLLEVENTS 20

/*处理连接*/
void handle_connection(int sockfd);
/*处理事件*/
void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf);
/*读处理*/
void do_read(int epollfd, int fd, int sockfd, char *buf);
/*写处理*/
void do_write(int epollfd, int fd, int sockfd, char *buf);
/*添加事件*/
void add_event(int epollfd, int fd, int state);
/*删除事件*/
void delete_event(int epollfd, int fd, int state);
/*修改事件*/
void modify_event(int epollfd, int fd, int state);

int main() {
    int socket_fd;
    struct sockaddr_in  socket_addr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(PORT);
    socket_addr.sin_addr.s_addr = inet_addr(IPADDR);
    connect(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr));
    //处理连接
    handle_connection(socket_fd);
    close(socket_fd);
    return 0;
}
/*处理连接*/
void handle_connection(int sockfd) {
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    char buf[MAXSIZE];
    int ret;
    epollfd = epoll_create(FDSIZE);
    add_event(epollfd, STDIN_FILENO, EPOLLIN);
    for ( ; ; ) {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, sockfd, buf);
    }
    close(epollfd);
}
/*处理事件*/
void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf) {
    int fd;
    int i;
    for (i = 0;i < num;i++) {
        fd = events[i].data.fd;
        if (events[i].events & EPOLLIN)
            do_read(epollfd, fd, sockfd, buf);
        else if (events[i].events & EPOLLOUT)
            do_write(epollfd, fd, sockfd, buf);
    }
}
/*读处理*/
void do_read(int epollfd, int fd, int sockfd, char *buf) {
    int nread;
    nread = read(fd,buf,MAXSIZE);
    if (nread == -1) {
        DBG("do_read->\033[31m❌\033[0m\n");
        fflush(stdout);
        close(fd);
    } else if (nread == 0) {
        DBG("do_read->\033[31m服务器关闭\033[0m\n");
        close(fd);
    } else {
        if (fd == STDIN_FILENO) {
            DBG("do_read->\033[32mread: %s\033[0m", buf);
            add_event(epollfd, sockfd, EPOLLOUT);
        } else {
            delete_event(epollfd, sockfd, EPOLLIN);
            add_event(epollfd, STDOUT_FILENO, EPOLLOUT);
        }
    }
}
/*写处理*/
void do_write(int epollfd, int fd, int sockfd, char *buf) {
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    if (nwrite == -1) {
        DBG("do_write->\033[31m❌\033[0m\n");
        fflush(stdout);
        close(fd);
    } else {
        if (fd == STDOUT_FILENO) {
            DBG("do_write->\033[32mwrite: %s\033[0m", buf);
            delete_event(epollfd, fd, EPOLLOUT);
        } else {
            DBG("do_write->\033[32mwrite: %s\033[0m", buf);
            modify_event(epollfd, fd, EPOLLIN);
        }
    }
    memset(buf, 0, MAXSIZE);
}
/*添加事件*/
void add_event(int epollfd, int fd, int state) {
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
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
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
