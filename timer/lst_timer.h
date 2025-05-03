#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    util_timer *timer;//指向客户连接对应的定时器，用于调整超时时间. 是否要默认初始化为NULL？
};

class util_timer//定时器类，封装一个定时事件
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire;//绝对超时时间
    
    void (* cb_func)(client_data *);//用于将用户连接对应的socket从epollfd上清除，同时close fd.
    client_data *user_data;
    util_timer *prev;
    util_timer *next;
};

class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer *timer);//添加一个定时器，用户新增客户连接时
    void adjust_timer(util_timer *timer);//调整一个定时器（用于活动连接延长超时时间）
    void del_timer(util_timer *timer);//删除一个定时器（用于读错误或者客户主动关闭连接时）
    void tick();//（SIGALRM信号触发时，从头检测升序定时器链表，将超时的定时器从链表删除，同时使用callback函数将客户fd从epollfd上清除，并close用户的fd）

private:
    void add_timer(util_timer *timer, util_timer *lst_head);//增加一个新的定时器，用于在头结点之后增加一个定时器，用作public add_timer和adjust_time的工具函数

    util_timer *head;
    util_timer *tail;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    //void addsig(int sig, void(handler)(int), bool restart = true);//handler前少了一个*
    void addsig(int sig, void(*handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;//触发SIGALRM的时间间隔
};

void cb_func(client_data *user_data);//这个声明不能省略，因为在webserver.cpp中引用了cb_func，在编译阶段webserver.cpp要确认有这个符号声明才能正确编译

#endif
