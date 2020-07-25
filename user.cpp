#include "user.h"

User::User(int sock_fd,const sockaddr_in& add)
{
    u_sock_fd=sock_fd;
    u_address=add;
}

void User::init(Shared_mem* sh,int ep_fd)
{
    shm=sh;
    shm->reset();
    u_epoll_fd=ep_fd;
}

//接收并处理客户端数据
void User::recv_process()
{}

//向客户端发送信息
void User::send_process()
{}