#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <cstring>
#include <cassert>

int sig_pipe_fd[2]; //用于处理信号的管道

//将fd文件描述符指定的文件设置为非阻塞
int set_nonblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    fcntl(fd,F_SETFL,old_option | O_NONBLOCK);
    
    return old_option;
}

//将某文件描述符加入到epoll的监听队列中
void add_fd(int epoll_fd,int fd)
{
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN | EPOLLET; //注册可读事件，设置ET模式
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);
    set_nonblocking(fd); //将该文件描述符设置为非阻塞
}

//从epoll_fd标识的epoll事件表中删除所有fd所注册的事件并关闭fd
void remove_fd(int epoll_fd,int fd)
{
    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,0);
    close(fd);
}

//信号处理函数
void sig_handler(int sig)
{
    int save_errno=errno;
    int msg=sig;
    send(sig_pipe_fd[1],(char*)&msg,1,0); //将信号转发给IO进程，统一事件源
    errno=save_errno; //还原进入函数之前的errono，保证函数的可重入性
}

//注册信号
void add_sig(int sig,void (*handler)(int),bool restart=true)
{
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler=handler; //将信号处理函数设置为handler参数
    if(restart)
    {
        sa.sa_flags|=SA_RESTART; //如果需要的话，设置当信号发生后重新调用被该信号终止的调用
    }
    sigfillset(&sa.sa_mask); //设置所有的信号掩码，即当执行信号处理函数时，不接收任何信号(信号在队列中等待)
    assert(sigaction(sig,&sa,NULL)!=-1);
}