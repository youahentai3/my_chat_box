#include "process_pool.h"

Process_pool::Process_pool(int _listen_fd,int _process_number):listen_fd(_listen_fd),process_number(_process_number),index(-1)
{
    assert(_process_number>0 && _process_number<=MAX_PROCESS_NUMBER);

    //创建process_number个子进程
    for(int i=0;i<process_number;i++)
    {
        sub_process.push_back(Process());

        //创建父子进程通信的管道
        int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sub_process[i].pipefd);
        assert(!ret);

        //创建子进程
        sub_process[i].pid=fork();
        assert(sub_process[i].pid>=0);

        if(sub_process[i].pid==0)
        {
            //子进程
            close(sub_process[i].pipefd[0]); //子进程通过1读写，关闭另一个管道口
            index=i;
            break;
        }
        else 
        {
            //父进程
            close(sub_process[i].pipefd[1]); //父进程通过0读写，关闭另一个管道口
            continue;
        }
    }
}