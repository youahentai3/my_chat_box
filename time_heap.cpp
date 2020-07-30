#include <unistd.h>
#include <iostream>
#include "user.h"

void default_call_back(User* _user)
{
    //std::cout<<"&&&&"<<std::endl;
    _user->status=false;
    //std::cout<<"status: "<<_user->status<<std::endl;
    _user->ask_process();
    //std::cout<<"status: "<<_user->status<<std::endl;
}

Heap_timer::Heap_timer(int delay,User* _h_user,std::function<void(User*)> _call_back):h_user(_h_user),call_back(_call_back)
{
    expire=time(NULL)+delay;
}

void Heap_timer::update_timer(int delay)
{
    expire=time(NULL)+delay;
}

time_t Heap_timer::get_expire()
{
    return expire;
}

void Heap_timer::set_call_back(std::function<void(User*)> _call_back)
{
    call_back=_call_back;
}

void Heap_timer::run_call_back()
{
    call_back(h_user);
}

bool compare_timer(std::shared_ptr<Heap_timer> timer1,std::shared_ptr<Heap_timer> timer2)
{
    return timer2->get_expire()<timer1->get_expire();
}

void Time_heap::add_timer(std::shared_ptr<Heap_timer> timer)
{
    heap.push_back(timer);
    std::push_heap(heap.begin(),heap.end(),compare_timer);
}

void Time_heap::del_timer(std::shared_ptr<Heap_timer> timer)
{
    auto ite=heap.begin();
    for(;ite!=heap.end();ite++)
    {
        if(*ite==timer)
            break;
    }
    if(ite!=heap.end())
        heap.erase(ite);
    std::make_heap(heap.begin(),heap.end(),compare_timer);
}

int Time_heap::get_top_delay()
{
    if(heap.empty())
        return DEFAULT_DELAY;
    else 
        return heap[0]->get_expire()-time(NULL);
}

void Time_heap::tick()
{
    std::make_heap(heap.begin(),heap.end(),compare_timer);
    while(!heap.empty())
    {
        //std::cout<<heap[0]->get_expire()<<" "<<time(NULL)<<std::endl;
        if(heap[0]->get_expire()>time(NULL))
            break;
        heap[0]->run_call_back();
        std::pop_heap(heap.begin(),heap.end(),compare_timer);
        heap.pop_back();
    } 
    alarm(get_top_delay());
}