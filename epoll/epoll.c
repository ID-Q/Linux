/*************************************************************************
	> File Name: epoll.c
	> Author: zhengdongqi
	> Mail: 1821260963@qq.com
	> Created Time: 一  4/ 1 13:14:17 2019
 ************************************************************************/
#include "epoll.h"

/*socket绑定*/
int socket_bind(int port) {
    int  listenfd;
    struct sockaddr_in socket_addr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        DBG("epoll.c->socket_bind\033[31m创建套接字失败: %s\033[0m\n", strerror(errno));
        close(listenfd);
        return -1;
    }
    //设置服务器
    memset(&socket_addr, 0, sizeof(socket_addr));//数据初始化清零
    socket_addr.sin_family = AF_INET;//设置协议族
    socket_addr.sin_port = htons(port);//端口
    socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址
    
    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        DBG("epoll.c->socket_bind\033[31m设置端口重用失败: %s\033[0m\n", strerror(errno));
        close(listenfd);
        return -1;
    }
    //绑定连接
    if (bind(listenfd, (struct sockaddr*)&socket_addr, sizeof(struct sockaddr)) < 0) {
        DBG("epoll.c->socket_bind\033[31m绑定失败: %s\033[0m\n", strerror(errno));
        close(listenfd);
        return -1;
    }
    return listenfd;
}
/*IO多路复用epoll*/
void do_epoll(int listenfd) {
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int ret;
    char buf[MAXSIZE] = 0;
    //创建一个描述符
    epollfd = epoll_create(FDSIZE);
    //添加监听描述符事件
    add_event(epollfd, listenfd, EPOLLIN);
    for ( ; ; )
    {
        //获取已经准备好的描述符事件
        ret = epoll_wait(epollfd, events, EPOLLEVENTS,-1);
        handle_events(epollfd, events, ret,l istenfd, buf);
    }
    close(epollfd);
}
/*事件处理函数*/
void
handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf) {
    int i;
    int fd;
    //进行选好遍历
    for (i = 0; i < num; i++)
    {
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
    struct sockaddr_in fdaddr;
    int len = sizeof(struct sockaddr_in), fd;
    fd = accept(listenfd,(struct sockaddr*)&fdaddr, (socklen_t *)&len));
    if (fd == -1) {
        DBG("epool.c->handle_accpet\033[31m接受套接字失败: %s\033[0m", strerror(errno));
    } else {
        DBG("epool.c->handle_accpet\033[31m检查到一个新用户: %s->%d\033[0m\n", inet_ntoa(fdaddr.sin_addr),fdaddr.sin_port);
        
        //添加一个客户描述符和事件
        add_event(epollfd, fd, EPOLLIN);
    }
}
/*处理进行连接*/
void handle_connect(int fd) {
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    char buf[MAXSIZE];
    int ret;
    epollfd = epoll_create(FDSIZE);
    add_event(epollfd, STDIN_FILENO, EPOLLIN);
    for ( ; ; ) {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, ret, fd, buf);
    }
    close(epollfd);
}
/*读处理*/
void do_read(int epollfd, int fd, char *buf)
{
    int nread;
    nread = read(fd, buf, MAXSIZE);
    if (nread == -1) {
        DBG("epool.c->do_read\033[31m读操作失败失败: %s\033[0m\n", strerror(errno));
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    } else if (nread == 0) {
        DBG("epool.c->do_read\033[DBG31m客户关闭\033[0m\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    } else {
        DBG("epool.c->do_read\033[DBG31mread：%s\033[0m\n", buf);
        //修改描述符对应的事件，由读改为写
        modify_event(epollfd, fd, EPOLLOUT);
    }
}
/*写处理*/
void do_write(int epollfd, int fd, char *buf)
{
    int nwrite;
    nwrite = write(fd, buf, strlen(buf));
    if (nwrite == -1) {
        DBG("epool.c->do_write\033[31m写操作失败失败: %s\033[0m\n", strerror(errno));
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    } else {
        modify_event(epollfd, fd, EPOLLIN);
    }
    memset(buf, 0, MAXSIZE);
}
/*添加事件*/
void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}
/*修改事件*/
void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}
/*删除事件*/
void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}
