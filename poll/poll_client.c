/*************************************************************************
 > File Name: poll_client.c
 > Author: zhengdongqi
 > Mail: 1821260963@qq.com
 > Created Time: Tue 09 Apr 2019 14:19:14 CST
 ************************************************************************/

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define MAXSIZE     1024
#define IPADDR      "192.168.1.44"
#define PORT        8731
/*处理连接*/
void handle_connect(int sockfd);

int main() {
    DBG("\033[33m送你一朵小花🌹\033[0m\n");
    fflush(stdout);
    int socket_fd;
    struct sockaddr_in  socket_addr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(PORT);
    socket_addr.sin_addr.s_addr = inet_addr(IPADDR);
    connect(socket_fd, (struct sockaddr*)&socket_addr,sizeof(socket_addr));
    //处理连接描述符
    handle_connect(socket_fd);
    return 0;
}
/*处理连接*/
void handle_connect(int socket_fd) {
    char sendline[MAXSIZE], recvline[MAXSIZE];
    struct pollfd pfds[2];
    int n;
    //添加连接描述符
    pfds[0].fd = socket_fd;
    pfds[0].events = POLLIN;
    //添加标准输入描述符
    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;
    for (; ;) {
        poll(pfds, 2, -1);
        if (pfds[0].revents & POLLIN) {
            n = read(socket_fd, recvline, MAXSIZE);
            if (n == 0) {
                DBG("handle_connect->\033[31m服务器已关闭\033[0m\n");
                close(socket_fd);
            }
            DBG("handle_connect->\033[33mrecvline: %s\033[0m", recvline);
            write(STDOUT_FILENO, recvline, n);
        }
        //测试标准输入是否准备好
        if (pfds[1].revents & POLLIN) {
            n = read(STDIN_FILENO, sendline, MAXSIZE);
            if (n == 0) {
                shutdown(socket_fd, SHUT_WR);
                continue;
            }
            DBG("handle_connect->\033[33msendline: %s\033[0m", sendline);
            write(socket_fd, sendline, n);
        }
    }
}
