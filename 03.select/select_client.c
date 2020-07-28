/*************************************************************************
 > File Name: select_client.c
 > Author: zhengdongqi
 > Mail: 1821260963@qq.com
 > Created Time: 一  4/ 8 17:36:23 2019
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#ifdef _DEBUG
#define DBG(fmt, args...) printf(fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define MAXLINE 1024
#define IPADDR "192.168.2.165"
#define PORT 8888
/*处理接收信息*/
void handle_recv_msg(int sockfd, char *buf)  {
    DBG("handle_recv_msg->\033[33m服务器发来消息: %s\033[0m\n", buf);
    sleep(5);
    write(sockfd, buf, strlen(buf) +1);
}


/*处理套接字连接*/
void handle_socket_connect(int fd) {
    char sendline[MAXLINE],recvline[MAXLINE];
    int maxfd = 0;
    fd_set rset;
    int n;
    struct timeval tv;
    int nfd = 0;
    
    while (1) {
        
        FD_ZERO(&rset);
        FD_SET(fd, &rset);
        maxfd = fd;
        
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        
        nfd = select(maxfd + 1, &rset, NULL, NULL, &tv);
        
        if (nfd == -1) {
            return ;
        }
        
        if (nfd == 0) {
            DBG("handle_socket_connect->\033[31mselect超时\033[0m\n");
            continue;
        }
        
        if (FD_ISSET(fd, &rset)) {
            n = read(fd, recvline, MAXLINE);
            if (n <= 0) {
                DBG("handle_socket_connect->\033[31m服务器已关闭\033[0m\n");
                close(fd);
                FD_CLR(fd, &rset);
                return;
            }
            handle_recv_msg(fd, recvline);
        }
    }
}

int main() {
    int socket_fd;
    struct sockaddr_in socket_addr;
    
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(PORT);
    socket_addr.sin_addr.s_addr = inet_addr(IPADDR);
    
    int nfd = 0;
    nfd = connect(socket_fd,(struct sockaddr*)&socket_addr, sizeof(socket_addr));
    if (nfd < 0) {
        DBG("\033[31m连接失败: %s\033[0m\n", strerror(errno));
        return -1;
    }
    
    DBG("\033[33m初次见面请多关照😄\033[0m\n");
    fflush(stdout);
    write(socket_fd, "hello server", 32);
    
    handle_socket_connect(socket_fd);
    
    return 0;
}
