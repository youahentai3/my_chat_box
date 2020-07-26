#include <sys/errno.h>
#include "rw_lock.h"

Rw_lock::Rw_lock()
{
    //创建一个信号量，表示该进程区域有多少进程未读取数据，当其为0时可写，>0时可读
    sem_id=semget(IPC_PRIVATE,1,0666);

    union semun sem_un;
    sem_un.val=0;
    semctl(sem_id,0,SETVAL,sem_un);
    //semctl(sem_id,1,SETVAL,sem_un);
    //sem_un.val=0;
    //semctl(sem_id,2,SETVAL,sem_un);
}

void Rw_lock::pv(int ind,int op)
{
    sembuf sem_b;
    sem_b.sem_num=ind;
    sem_b.sem_op=op;
    sem_b.sem_flg=SEM_UNDO;
    while(semop(sem_id,&sem_b,1)==-1)
    {
        if(errno==EINTR)
            continue;
        break;
    }
}

void Rw_lock::r_lock()
{
    pv(0,-1);
    /*pv(1,-1); //获取读信号量
    union semun sem_un;
    semctl(sem_id,2,GETVAL,sem_un); //获取读者数
    if(sem_un.val==parts)
        pv(0,-1); //获取写信号量
    pv(2,1); //读者数加一
    pv(1,1); //释放读信号量*/
}

void Rw_lock::r_unlock()
{
    pv(0,-1); //获取读信号量
    //pv(2,-1); //读者数减一
    /*union semun sem_un;
    semctl(sem_id,2,GETVAL,sem_un); //获取读者数
    if(sem_un.val==parts)
        pv(0,1); //释放写信号量
    pv(1,1); //释放读信号量*/
}

void Rw_lock::w_lock()
{
    pv(0,0); //获取写信号量
}

void Rw_lock::w_unlock(int num)
{
    /*pv(1,-1);
    union semun sem_un;
    sem_un.val=num;
    semctl(sem_id,2,SETVAL,sem_un);
    pv(1,1);*/
    //pv(0,1); //释放写信号量s
    union semun sem_un;
    sem_un.val=num*2;
    semctl(sem_id,0,SETVAL,sem_un);
}