#include <cassert>
#include <unistd.h>
#include <cstring>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include "shared_mem.h"

const char* Shared_mem::shm_name="/my_shm";

Shared_mem::Shared_mem(int num,int id):parts(num),ind(id),lock(new Rw_lock[num])
{
    assert(num<=PROCESS_NUM_LIMIT);
    shm_fd=shm_open(shm_name,O_CREAT | O_RDWR,0666); //创建打开用于共享内存的文件
    assert(shm_fd!=-1);
    int ret=ftruncate(shm_fd,parts*BUFFER_SIZE); //设置共享内存大小为进程数×缓存区大小
    assert(ret!=-1);
}

Shared_mem::~Shared_mem()
{
    //取消共享内存在该进程的地址空间的映射
    munmap((void*)shm_m,parts*BUFFER_SIZE);
}

void Shared_mem::set_ind(int id)
{
    ind=id;
}

void Shared_mem::connect()
{
    //将共享内存映射到当前进程地址空间
    shm_m=(char*)mmap(NULL,parts*BUFFER_SIZE,PROT_READ | PROT_WRITE,MAP_SHARED,shm_fd,0);
    assert(shm_m!=MAP_FAILED);
    close(shm_fd);
}

void Shared_mem::unlink()
{
    //将共享内存标记为等待删除
    shm_unlink(shm_name);
}

void Shared_mem::reset()
{
    //将当前进程所对应的共享内存区域清空
    memset(shm_m+ind*BUFFER_SIZE,0,BUFFER_SIZE);
}

void Shared_mem::write_in(char* msg) //需保证msg是BUFFER_SIZE大小
{
    lock[ind].w_lock();
    reset();
    //std::cout<<msg<<std::endl;
    memcpy(shm_m+BUFFER_SIZE*ind,msg,BUFFER_SIZE);
    lock[ind].w_unlock(parts);
}

void Shared_mem::read_out(int id,char* buffer) //buffer需是分配了空间的数组
{
    lock[ind].r_lock();
    memcpy(buffer,shm_m+BUFFER_SIZE*id,BUFFER_SIZE);
    lock[ind].r_unlock();
}