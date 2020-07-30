#ifndef USER_H
#define USER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory>
#include <ctime>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>
#include "shared_mem.h"

#define DEFAULT_DELAY 10

class Heap_timer;

class User
{
private:
    static std::shared_ptr<Shared_mem> shm; //进程的共享内存
    static int u_epoll_fd; //该进程所有用户所在的epoll id
    int u_sock_fd; //该用户的socket id
    sockaddr_in u_address; //该用户的socket地址
    int u_read_index;
    bool is_used;
public:
    std::shared_ptr<Heap_timer> timer;
    bool status; //表示状态，true为正常状态，0为挂起状态：不向该客户端发送信息
public:
    User()=default;
    User(int sock_fd,const sockaddr_in& add);
    bool recv_process();
    void send_process(char* buffer);
    bool get_is_used();
    void ask_process();
    void set_timer(std::shared_ptr<Heap_timer> _timer);
    static void init(std::shared_ptr<Shared_mem> sh,int ep_fd);
    ~User()=default;
};

void default_call_back(User*);

class Heap_timer
{
private:
    time_t expire; //定时器生效的绝对时间
    User* h_user;
    std::function<void(User*)> call_back;
public:
    Heap_timer(int delay,User* _h_user,std::function<void(User*)> _call_back=default_call_back);
    time_t get_expire();
    void set_call_back(std::function<void(User*)> _call_back);
    void run_call_back();
    void update_timer(int delay=DEFAULT_DELAY);
};

class Time_heap
{
private:
    std::vector<std::shared_ptr<Heap_timer>> heap;
public:
    Time_heap()=default;
    ~Time_heap()=default;
    void add_timer(std::shared_ptr<Heap_timer> timer);
    void del_timer(std::shared_ptr<Heap_timer> timer);
    int get_top_delay();
    void pop_timer();
    void tick();
};

#endif