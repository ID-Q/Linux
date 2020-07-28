/*************************************************************************
 > File Name: poll_server.c
 > Author: zhengdongqi
 > Mail: 1821260963@qq.com
 > Created Time: Tue 09 Apr 2019 14:15:43 CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
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
#define OPEN_MAX    1000
#define INFTIM      -1

/*函数声明*/
/*创建套接字并进行绑定*/
int socket_bind(int port);
/*IO多路复用poll*/
void do_poll(int listenfd);
/*处理多个连接*/
void handle_connect(struct pollfd *pfds, int num);

int main() {
    DBG("\033[33m想要小🌹吗?\033[0m\n");
    fflush(stdout);
    int  listenfd;
    struct sockaddr_in socket_addr;
    listenfd = socket_bind(PORT);
    listen(listenfd, LISTENQ);
    do_poll(listenfd);
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
/*IO多路复用破poll*/
void do_poll(int listenfd) {
    int poll_fd;
    struct sockaddr_in poll_addr;
    struct pollfd pfds[OPEN_MAX];
    int maxfd;
    int i;
    int nready;
    //添加监听描述符
    pfds[0].fd = listenfd;
    pfds[0].events = POLLIN;
    //初始化客户连接描述符
    for (i = 1; i < OPEN_MAX; i++) {
        pfds[i].fd = -1;
    }
    maxfd = 0;
    //循环处理
    for ( ; ; ) {
        //获取可用描述符的个数
        nready = poll(pfds, maxfd+1, INFTIM);
        if (nready == -1) {
            DBG("do_poll->\033[31mpoll error\033[0m\n");
            return ;
        }
        //测试监听描述符是否准备好
        if (pfds[0].revents & POLLIN) {
            socklen_t len = sizeof(poll_addr);
            //接受新的连接
            if ((poll_fd = accept(listenfd, (struct sockaddr*)&poll_addr, &len)) == -1) {
                if (errno == EINTR)
                    continue;
                else {
                    DBG("do_poll->\033[31maccept error\033[0m\n");
                    return ;
                }
            }
            DBG("do_poll->\033[33maccept a new client: %s->%d\033[0m\n", inet_ntoa(poll_addr.sin_addr), ntohs(poll_addr.sin_port));
            //将新的连接描述符添加到数组中
            for (i = 1; i < OPEN_MAX; i++) {
                if (pfds[i].fd < 0) {
                    pfds[i].fd = poll_fd;
                    break;
                }
            }
            if (i == OPEN_MAX) {
                DBG("\033[34m累死了😢\033[0m\n");
                return ;
            }
            //将新的描述符添加到读描述符集合中
            pfds[i].events = POLLIN;
            //记录客户连接套接字的个数
            maxfd = (i > maxfd ? i : maxfd);
            if (--nready <= 0)
                continue;
        }
        //处理客户连接
        handle_connect(pfds,maxfd);
    }
}

void handle_connect(struct pollfd *pfds, int num) {
    int i, n;
    char buf[MAXSIZE];
    memset(buf, 0, MAXSIZE);
    for (i = 1; i <= num; i++) {
        if (pfds[i].fd < 0)
            continue;
        //测试客户描述符是否准备好
        if (pfds[i].revents & POLLIN) {
            //接收客户端发送的信息
            n = read(pfds[i].fd, buf, MAXSIZE);
            if (n == 0) {
                close(pfds[i].fd);
                pfds[i].fd = -1;
                continue;
            }
            DBG("handle_connect->\033[33mread msg is: %s\033[0m", buf);
            write(STDOUT_FILENO, buf, n);
            //向客户端发送buf
            write(pfds[i].fd, buf, n);
        }
    }
}
