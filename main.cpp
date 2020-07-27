#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include "process_pool.h"

int main(int argc,char** argv)
{
    if(argc<=2)
    {
        std::cout<<"error input"<<std::endl;
        return 1;
    }
    const char* ip=argv[1]; //ip地址
    int port=atoi(argv[2]); //端口号，从字符串转化为int值

    int listen_fd=socket(PF_INET,SOCK_STREAM,0); //创建服务端socket
    assert(listen_fd>=0);

    int ret=0;
    sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr); //将ip地址转换为网络字节序
    address.sin_port=htons(port); //端口号转换为网络字节序

    ret=bind(listen_fd,(sockaddr*)&address,sizeof(address)); //将socket和一个socket地址关联起来
    assert(ret!=-1);

    ret=listen(listen_fd,10000);
    assert(ret!=-1);

    std::cout<<"eeeee"<<std::endl;
    Process_pool::create(listen_fd,8);
    Process_pool::begin();
    std::cout<<"eeeee"<<std::endl;

    close(listen_fd);
    return 0;
}