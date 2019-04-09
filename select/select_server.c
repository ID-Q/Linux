/*************************************************************************
 > File Name: select_server.c
 > Author: zhengdongqi
 > Mail: 1821260963@qq.com
 > Created Time: 一  4/ 8 16:04:11 2019
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define IPADDR "192.168.2.165"
#define PORT 8888
#define MAXSIZE 1024
#define LISTENQ 5
#define SIZE 10

/*定义一个结构体存放数据信息*/
typedef struct date_t {
    int cli_cnt; /*客户数量*/
    int cli_fds[SIZE]; /*客户套接字*/
    fd_set allfds; /*句柄集合*/
    int maxfd; /*句柄最大值*/
}date_t;
date_t *cli_d = NULL;

/*函数声明*/
/*创建一个socket 类型TCP*/
int socket_create_tcp(int port);
/*创建一个接收函数*/
int socket_accept_tcp(int sockfd);
/*处理收到的消息*/
int handle_recv_msg(int fd, char *buf);s
/*接收消息*/
void socket_recv_msg(fd_set *rset);
/*select处理函数*/
void handle_select_proc(int sockfd);
/*创建一个socket 类型TCP*/
/*销毁数据*/
void date_destroy();
/*初始化*/
int socket_init();

int main() {
    DBG("\033[33m很高兴为您服务💁\033[0m\n");
    fflush(stdout);
    int fd;
    /*初始化服务端*/
    if (socket_init() < 0) {
        return -1;
    }
    /*创建服务,开始监听客户端请求*/
    fd = socket_create_tcp(PORT);
    if (fd < 0) {
        DBG("\033[31m套接字创建失败\033[0m\n");
        return -1;
    }
    /*开始接收并处理客户端请求*/
    handle_select_proc(fd);
    date_destroy();
    return 0;
}
/*创建一个socket 类型TCP*/
int socket_create_tcp(int port) {
    int socket_fd;
    struct sockaddr_in socket_addr;
    //创建套接字
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        DBG("socket_create_tcp->\033[31m创建套接字失败: %s\033[0m\n", strerror(errno));
        close(socket_fd);
        return -1;
    }
    //设置服务器
    memset(&socket_addr, 0, sizeof(socket_addr));//数据初始化清零
    socket_addr.sin_family = AF_INET;//设置协议族
    socket_addr.sin_port = htons(port);//端口
    socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址
    //端口重用
    int reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        DBG("socket_create_tcp->\033[31m设置端口重用失败: %s\033[0m\n", strerror(errno));
        close(socket_fd);
        return -1;
    }
    //绑定连接
    if (bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(struct sockaddr)) < 0) {
        DBG("socket_create_tcp->\033[31m绑定失败: %s\033[0m\n", strerror(errno));
        close(socket_fd);
        return -1;
    }
    //设置监听
    if (listen(socket_fd, 20) < 0) {
        DBG("socket_create_tcp->\033[31m监听失败: %s\033[0m\n", strerror(errno));
        close(socket_fd);
        return -1;
    }
    return socket_fd;
}
/*创建一个接收函数*/
int socket_accept_tcp(int sockfd) {
    int afd = -1;
    struct sockaddr_in accept_addr;
    int len = sizeof(accept_addr);
    
    afd = accept(sockfd, (struct sockaddr *)&accept_addr, (socklen_t *)&len);
    
    if (afd == -1) {
        DBG ("socket_accept_tcp->\033[31m接收失败: %s\033[0m\n", strerror(errno));
        return -1;
    }
    
    //将新的连接描述符添加到数组中
    int i = 0;
    for (i = 0; i < SIZE; i++) {
        if (cli_d->cli_fds[i] < 0) {
            cli_d->cli_fds[i] = afd;
            cli_d->cli_cnt++;
            break;
        }
    }
    
    if (i == SIZE) {
        DBG("socket_accept_tcp->\033[31m太多客户了 好累啊😢\033[0m\n");
        fflush(stdout);
        return -1;
    }
    return 0;
}
/*处理收到的消息*/
int handle_recv_msg(int fd, char *buf) {
    assert(buf);
    printf("handle_recv_msg->\033[32mrecv buf is :%s\033[0m\n", buf);
    write(fd, buf, strlen(buf) +1);
    return 0;
}
/*接收消息*/
void socket_recv_msg(fd_set *rset) {
    int i = 0, n = 0;
    int fd;
    char buf[MAXSIZE] = {0};
    for (i = 0; i <= cli_d->cli_cnt; i++) {
        fd = cli_d->cli_fds[i];
        if (fd < 0) {
            continue;
        }
        /*判断客户端套接字是否有数据*/
        if (FD_ISSET(fd, rset)) {
            //接收客户端发送的信息
            n = read(fd, buf, MAXSIZE);
            if (n <= 0) {
                /*n==0表示读取完成，客户都关闭套接字*/
                FD_CLR(fd, &cli_d->allfds);
                close(fd);
                cli_d->cli_fds[i] = -1;
                continue;
            }
            handle_recv_msg(fd, buf);
        }
    }
}
/*select处理函数*/
void handle_select_proc(int sockfd) {
    int fd = -1;
    int nfd = 0;
    fd_set *rset = &cli_d->allfds;
    struct timeval tv;
    int i = 0;
    
    while (1) {
        /*每次调用select前都要重新设置文件描述符和时间，因为事件发生后，文件描述符和时间都被内核修改啦*/
        FD_ZERO(rset);
        /*添加监听套接字*/
        FD_SET(sockfd, rset);
        cli_d->maxfd = sockfd;
        
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        
        /*添加客户端套接字*/
        for (i = 0; i < cli_d->cli_cnt; i++) {
            fd = cli_d->cli_fds[i];
            /*去除无效的客户端句柄*/
            if (fd != -1) {
                FD_SET(fd, rset);
            }
            cli_d->maxfd = (fd > cli_d->maxfd ? fd : cli_d->maxfd);
        }
        
        /*开始轮询接收处理服务端和客户端套接字*/
        nfd = select(cli_d->maxfd + 1, rset, NULL, NULL, &tv);
        if (nfd == -1) {
            DBG("handle_socket_select->\033[31mselect失败: %s\033[0m\n", strerror(errno));
            return ;
        }
        if (nfd == 0) {
            DBG("handle_socket_select->\033[31mselect超时: %s\033[0m\n", strerror(errno));
            continue;
        }
        if (FD_ISSET(sockfd, rset)) {
            /*监听客户端请求*/
            socket_accept_tcp(sockfd);
        } else {
            /*接受处理客户端消息*/
            socket_recv_msg(rset);
        }
    }
    return ;
}
/*销毁数据*/
void date_destroy() {
    if (cli_d) {
        free(cli_d);
        cli_d = NULL;
    }
    return ;
}
/*初始化*/
int socket_init() {
    cli_d = (date_t *)malloc(sizeof(date_t));
    if (cli_d == NULL) {
        return -1;
    }
    
    memset(cli_d, 0, sizeof(date_t));
    
    int i = 0;
    for (; i < SIZE; i++) {
        cli_d->cli_fds[i] = -1;
    }
    return 0;
}
