#ifndef TIME_HEAP_H
#define TIME_HEAP_H

/*#include <ctime>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>
#include "user.h"

#define DEFAULT_DELAY 30

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
};*/

#endif