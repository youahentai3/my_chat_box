#ifndef USER_H
#define USER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory>
#include "shared_mem.h"

class User
{
private:
    static std::shared_ptr<Shared_mem> shm; //进程的共享内存
    static int u_epoll_fd; //该进程所有用户所在的epoll id
    int u_sock_fd; //该用户的socket id
    sockaddr_in u_address; //该用户的socket地址
    int u_read_index;
public:
    User()=default;
    User(int sock_fd,const sockaddr_in& add);
    void recv_process();
    void send_process(int id);
    static void init(std::shared_ptr<Shared_mem> sh,int ep_fd);
    ~User()=default;
};
std::shared_ptr<Shared_mem> User::shm=nullptr;
int User::u_epoll_fd=-1;

#endif