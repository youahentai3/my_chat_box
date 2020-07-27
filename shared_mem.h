#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <memory>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rw_lock.h"

class Shared_mem
{
private:
    int shm_fd; //共享内存标识符
    int parts; //划分为多少个区域，每个进程独享一个区域
    int ind; //当前进程的序号
    char* shm_m;
    std::unique_ptr<Rw_lock[]> lock;
    static const int PROCESS_NUM_LIMIT=17; //最大共享内存进程数
    static const char* shm_name;
public:
    static const int BUFFER_SIZE=1024; //缓存区大小
public:
    Shared_mem()=default;
    Shared_mem(int num,int id);
    ~Shared_mem();
    void set_ind(int id);
    void connect(); //将共享内存映射到当前进程的内存空间中
    void unlink();
    void reset();
    void write_in(char* msg);
    void read_out(int id,char* buffer);
};

#endif