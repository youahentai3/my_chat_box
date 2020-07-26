#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>
#include <iostream>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unordered_map>
#include "process_pool.h"
#include "event_handle.h"
#include "user.h"
#include "shared_mem.h"

Process_pool::Process_pool(int _listen_fd,int _process_number):listen_fd(_listen_fd),process_number(_process_number),index(-1),sh_m(new Shared_mem(process_number,0))
{
    assert(_process_number>0 && _process_number<=MAX_PROCESS_NUMBER);

    //创建process_number个子进程
    for(int i=0;i<process_number;i++)
    {
        sub_process.push_back(Process());

        //创建父子进程通信的管道
        int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sub_process[i].pipefd);
        assert(!ret);
        ret=socketpair(PF_UNIX,SOCK_STREAM,0,sub_process[i].an_pipefd);
        assert(!ret);

        //创建子进程
        sub_process[i].pid=fork();
        assert(sub_process[i].pid>=0);

        if(sub_process[i].pid==0)
        {
            //子进程
            close(sub_process[i].pipefd[0]); //子进程通过1读写，关闭另一个管道口
            close(sub_process[i].an_pipefd[0]);
            index=i;
            sh_m->set_ind(index); //设置共享内存序号
            break;
        }
        else 
        {
            //父进程
            close(sub_process[i].pipefd[1]); //父进程通过0读写，关闭另一个管道口
            close(sub_process[i].an_pipefd[1]);
            continue;
        }
    }
}

//统一事件源
void Process_pool::setup_sig_pipe()
{
    epoll_fd=epoll_create(5); //向内核申请创建epoll事件表
    assert(epoll_fd!=-1);

    int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sig_pipe_fd); //建立信号处理管道
    assert(ret!=-1);

    set_nonblocking(sig_pipe_fd[1]); //将写端口设置为非阻塞
    add_fd(epoll_fd,sig_pipe_fd[0]); //向epoll事件表注册读端口

    //注册信号量
    add_sig(SIGCHLD,sig_handler,true); //子进程退出或暂停
    add_sig(SIGTERM,sig_handler,true); //终止进程
    add_sig(SIGINT,sig_handler,true); //键盘输入中断
    add_sig(SIGPIPE,SIG_IGN,true); //往读端被关闭的管道或socket连接中写数据，SIG_IGN表示忽略该信号
}

void Process_pool::run_child()
{
    setup_sig_pipe(); //统一事件源并创建epoll时间表

    add_fd(epoll_fd,sub_process[index].pipefd[1]); //注册监听与父进程通信的管道口
    epoll_event events[MAX_EVENT_NUMBER];
    User::init(sh_m,epoll_fd);
    std::vector<User> users;
    std::unordered_map<int,User> users_map;
    int number=0,ret=-1;

    while(is_stop)
    {
        number=epoll_wait(epoll_fd,events,MAX_EVENT_NUMBER,-1);
        if(number<0 && errno!=EINTR) //EINTR表示epoll_wait被更高级的系统调用打断，可以重新调用
        {
            std::cout<<"epoll failure"<<std::endl; //后面输出到日志
            break;
        }

        for(int i=0;i<number;i++)
        {
            int sock_fd=events[i].data.fd;
            if(sock_fd==sub_process[index].pipefd[1] && (events[i].events & EPOLLIN)) //与父进程的通信管道可读
            {
                int client=0;
                ret=recv(sock_fd,(char*)&client,sizeof(client),0);
                if((ret<0 && errno!=EAGAIN) || ret==0)
                {
                    continue;
                }
                else 
                {
                    struct sockaddr_in client_address;
                    socklen_t client_addr_length=sizeof(client_address);
                    int connfd=accept(listen_fd,(struct sockaddr*)&client_address,&client_addr_length);
                    if(connfd<0)
                    {
                        continue;
                    }
                    add_fd(epoll_fd,connfd);
                    User user(connfd,client_address);
                    users_map.insert(std::make_pair(connfd,user));
                    //users[connfd]->init();
                }
            }
            else if(sock_fd==sub_process[index].an_pipefd[1] && (events->events & EPOLLIN))
            {
                ;
            }
            else if(sock_fd==sig_pipe_fd[0] && (events->events & EPOLLIN)) //处理信号
            {
                int sig;
                char signals[1024];
                ret=recv(sig_pipe_fd[0],signals,sizeof(signals),0);
                if(ret<=0)
                {
                    continue;
                }
                else 
                {
                    for(int i=0;i<ret;i++)
                    {
                        if(signals[i]==SIGCHLD)
                        {
                            pid_t t_pid;
                            int stat;
                            while((t_pid=waitpid(-1,&stat,WNOHANG))>0)
                            {
                                continue;
                            }
                        }
                        else if(signals[i]==SIGTERM || signals[i]==SIGINT)
                        {
                            is_stop=true;
                        }
                    }
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                //客户端数据到来
                users_map[sock_fd].recv_process();
                //发送通知给父进程，父进程负责把消息发给其余进程
                int id=index;
                send(sub_process[index].an_pipefd[1],(char*)&id,sizeof(id),0);
            }
        }
    }

    close(sub_process[index].an_pipefd[1]);
    close(sub_process[index].pipefd[1]);
    close(epoll_fd);
}

void Process_pool::run_parent()
{
    setup_sig_pipe();

    add_fd(epoll_fd,listen_fd);

    epoll_event events[MAX_EVENT_NUMBER];
    int sub_process_counter=0;
    int new_conn=1;
    int ret=-1;

    while(!is_stop)
    {
        int number=epoll_wait(epoll_fd,events,MAX_EVENT_NUMBER,-1);
        if(number<0 && errno!=EINTR)
        {
            continue;
        }

        for(int i=0;i<number;i++)
        {
            int sock_fd=events[i].data.fd;
            int k=sub_process_counter;
            if(sock_fd==listen_fd)
            {
                do
                {
                    if(sub_process[k].pid!=-1) //该子进程有效则分配
                        break;
                    k=(k+1)%process_number;
                }
                while(k!=sub_process_counter); //所有子进程都无效，从这里退出循环

                if(sub_process[k].pid==-1)
                {
                    is_stop=true; //所有子进程都无效，无法分配进程，父进程停止
                    break;
                }

                sub_process_counter=(k+1)%process_number;
                send(sub_process[k].pipefd[0],(char*)&new_conn,sizeof(new_conn),0); //将连接分配给子进程，随后该连接的所有操作由子进程负责
            }
            else if(sock_fd==sig_pipe_fd[0] && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret=recv(sig_pipe_fd[0],signals,sizeof(signals),0);

                if(ret<=0)
                {
                    continue;
                }
                else 
                {
                    for(int i=0;i<ret;i++)
                    {
                        if(signals[i]==SIGCHLD)
                        {
                            pid_t t_pid;
                            int stat;
                            while((t_pid=waitpid(-1,&stat,WNOHANG))>0)
                            {
                                for(int j=0;j<process_number;j++)
                                {
                                    if(sub_process[j].pid==t_pid) //该子进程退出
                                    {
                                        //关闭与退出的子进程之间的通信管道，并将其pid设为-1
                                        close(sub_process[j].pipefd[0]);
                                        close(sub_process[j].an_pipefd[0]);
                                        sub_process[j].pid=-1;
                                    }
                                }
                            }

                            is_stop=true;
                            for(int j=0;j<process_number;j++)
                            {
                                if(sub_process[j].pid!=-1)
                                {
                                    is_stop=false;
                                    break;
                                }
                            }
                        }
                        else if(signals[i]==SIGTERM || signals[i]==SIGINT)
                        {
                            //若父进程收到终止信号，向所有有效子进程发送终止信号
                            for(int j=0;j<process_number;j++)
                            {
                                int t_pid=sub_process[j].pid;
                                if(t_pid!=-1)
                                {
                                    kill(t_pid,SIGTERM);
                                }
                            }
                        }
                    }
                }
            }
            else if(sock_fd==sub_process[i].an_pipefd[0] && (events->events & EPOLLIN))
            {
                //子进程有信息传来，表示接收到客户端信息
                int id;
                int ret=recv(sock_fd,(char*)&id,sizeof(id),0);
                for(int i=0;i<process_number;i++)
                {
                    if(sub_process[i].pid!=-1)
                    {
                        send(sub_process[i].an_pipefd[0],(char*)id,sizeof(id),0);
                    }
                }
            }
        }
    }

    close(epoll_fd);
}