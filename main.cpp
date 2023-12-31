#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>

#include "mycoro/task.h"
#include "mycoro/awaiters.h"
#include "mycoro/async.h"
#include "mycoro/chan.h"

using namespace std::chrono_literals;

mycoro::Channel<int> ch(1);

mycoro::Task<void> cofunc1()
{
    std::cout << "start cofunc1" << std::endl;
    co_await mycoro::delay(1s);
    std::cout << "cofunc1 after delay 1s" << std::endl;
    co_await ch.put(111);
    std::cout << "cofunc1 put 111" << std::endl;
    co_await mycoro::delay(1s);
    std::cout << "cofunc1 after delay 1s" << std::endl;
    co_await ch.put(222);
    std::cout << "cofunc1 put 222" << std::endl;
    std::cout << "end cofunc1" << std::endl;
}

mycoro::Task<int> cofunc2()
{
    std::cout << "start cofunc2" << std::endl;
    co_await mycoro::delay(2s);
    std::cout << "cofunc2 after delay 2s" << std::endl;
    auto a = co_await ch.get();
    std::cout << "cofunc2 get "<< a << std::endl;
    co_await mycoro::delay(2s);
    std::cout << "cofunc2 after delay 2s" << std::endl;
    auto b = co_await ch.get();
    std::cout << "cofunc2 get "<< b << std::endl;
    co_return a+b;
    std::cout << "end cofunc2" << std::endl;
}

mycoro::Task<int> cofunc()
{
    std::cout << "start" << std::endl;

    auto task1 = cofunc1();
    auto task2 = cofunc2();

    std::cout << "task1 and task2 created" << std::endl;
    co_await std::move(task1);
    std::cout << "task1 done" << std::endl;
    auto b = co_await std::move(task2);
    std::cout << "task2 done" << std::endl;
    co_return b;
    std::cout << "end" << std::endl;
}

mycoro::Task<int> comain()
{
    auto res = co_await cofunc();
    std::cout << "cofunc done" << std::endl;
    co_await mycoro::async( [](){ std::this_thread::sleep_for(5s); std::cout << "run in async" << std::endl; } );
    std::cout << "async done" << std::endl;
    co_return res;
}

int main(int, char**){
    auto &sch = mycoro::Scheduler::instance();
    
    auto task = comain();
    
    while(sch.size() > 0)
    {
        sch.dispatch();
        std::this_thread::sleep_for(50ms);
    }

    std::cout << task.get_result() << std::endl;
}
