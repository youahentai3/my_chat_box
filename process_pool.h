#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include <unistd.h>
#include <memory>
#include <cassert>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>

class Process_pool;

//进程类，pid是进程id，pipefd是该进程与父进程交换信息的管道描述符
class Process
{
public:
    Process():pid(-1){}
    friend class Process_pool;
private:
    pid_t pid;
    int pipefd[2];
    int an_pipefd[2];
};

class Process_pool
{
private:
    static const int MAX_PROCESS_NUMBER=16; //进程池的最大进程数
    static const int USER_PER_PROCESS=65536; //每个进程最多能处理的客户链接数量
    static const int MAX_EVENT_NUMBER=10000; //epoll能处理的最大事件数
    int process_number; //进程池的进程数
    int index; //进程在进程池中的序号(从零开始)
    int epoll_fd; //epoll标识符
    int listen_fd;
    bool is_stop; //标识进程是否需要运行
    std::vector<Process> sub_process; //进程的描述信息
    std::shared_ptr<Shared_mem> sh_m; //共享内存实例
    static std::unique_ptr<Process_pool> instance; //进程池的静态实例
private:
    Process_pool(int _listen_fd,int _process_number=8);
    void setup_sig_pipe();
    void run_parent();
    void run_child();
public:
    static void create(int _listen_fd,int _process_number=8)
    {
        if(!instance)
        {
            instance.reset(new Process_pool(_listen_fd,_process_number));
        }
    }
    ~Process_pool()=default;
};
std::unique_ptr<Process_pool> Process_pool::instance=nullptr;

#endif