#ifndef RW_LOCK_H
#define RW_LOCK_H

#include <sys/sem.h>

union semun
{
    int val;
    semid_ds* buf;
    unsigned short* array;
    seminfo* _buf;
};

//读写锁
class Rw_lock
{
private:
    int sem_id;
    int an_sem_id;
    int parts;
private:
    void pv(int ind,int op);
    void an_pv(int ind,int op);
public:
    Rw_lock();
    void r_lock();
    void r_unlock();
    void w_lock();
    void w_unlock(int num);
};

#endif