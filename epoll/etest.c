/*************************************************************************
	> File Name: etest.c
	> Author: zhengdongqi
	> Mail: 1821260963@qq.com
	> Created Time: 四  4/ 4 16:24:45 2019
 ************************************************************************/

#include "epoll.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT        9006
#define MAXSIZE     1024
#define LISTENQ     20
#define FDSIZE      1000
#define EPOLLEVENTS 100

int main() {
	int  listenfd;
    listenfd = socket_bind(PORT);
	while(1) {
		listen(listenfd, LISTENQ);
    	do_epoll(listenfd);
	}
    return 0;
}

