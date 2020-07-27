#include <cstring>
#include <iostream>
#include "user.h"
#include "event_handle.h"

std::shared_ptr<Shared_mem> User::shm=nullptr;
int User::u_epoll_fd=-1;

User::User(int sock_fd,const sockaddr_in& add)
{
    u_sock_fd=sock_fd;
    u_address=add;
    u_read_index=0;
    is_used=true;
}

void User::init(std::shared_ptr<Shared_mem> sh,int ep_fd)
{
    shm=sh;
    shm->reset();
    u_epoll_fd=ep_fd;
}

//接收并处理客户端数据
bool User::recv_process()
{
    if(!is_used)
        return false;

    int ret=-1;
    char buffer[Shared_mem::BUFFER_SIZE];
    char cbuffer[Shared_mem::BUFFER_SIZE];

    memset(buffer,0,Shared_mem::BUFFER_SIZE);

    //LT模式
    ret=recv(u_sock_fd,buffer,Shared_mem::BUFFER_SIZE-1,0);
    if(ret<0)
        return false;
    else if(ret==0)
    {
        //对方关闭连接
        remove_fd(u_epoll_fd,u_sock_fd); //注销该socket
        is_used=false;
        return false;
    }
    else 
    {
        //正常接收到信息
        shm->write_in(buffer);
        u_read_index=0;
        return true;
    }

    //因为是ET模式，所以需要一次性把该次读取事件所有的数据全部读取
    /*while(u_read_index<Shared_mem::BUFFER_SIZE)
    {
        //memset(cbuffer,0,Shared_mem::BUFFER_SIZE);
        ret=recv(u_sock_fd,buffer+u_read_index,Shared_mem::BUFFER_SIZE-1-u_read_index,0);

        if(ret<0)
        {
            //数据读取完毕
            if(errno==EAGAIN || errno==EWOULDBLOCK)
            {
                break;
            }
            //暂时无数据可读，退出
            //close(u_sock_fd);
            break;
        }
        else if(ret==0)
        {
            //对方关闭连接
            remove_fd(u_epoll_fd,u_sock_fd); //注销该socket
            is_used=true;
            break;
        }
        else 
        {
            //std::cout<<buffer<<std::endl;
            u_read_index+=ret;
        }
    }
    if(u_read_index)
    {
        //std::cout<<buffer<<std::endl;
        shm->write_in(buffer);
        u_read_index=0;
        return true;
    }*/
    return false;
}

//向客户端发送信息
void User::send_process(char* buffer)
{
    //char buffer[Shared_mem::BUFFER_SIZE];
    if(!is_used)
        return;
   // shm->read_out(id,buffer);
    send(u_sock_fd,buffer,strlen(buffer)+1,0);
}

//该user是否可用
bool User::get_is_used()
{
    return is_used;
}