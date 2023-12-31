# My Coroutine Lib

C++协程库，编写的目的仅用于学习和理解C++20的coroutines的逻辑，不可用于生产。

强烈推荐**霍丙乾** https://github.com/bennyhuo 的《渡劫 C++ 协程》教程，我这个协程库受到这位作者的强烈启发。

## 使用方法

```cpp
#include <iostream>
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

```

## 原理

`Task` ：实现coroutine的基本task，即`promise_type`的实现。并且使用`await_transform`来支持`co_await task`的操作。

`Scheduler`：“手动”协程调度器，这个手动的意思是需要循环调用`dispatch()`方法来实现调度。在`dispatch()`方法中，依次检查每个协程是否可以恢复，如果可以则恢复之。

各种`Awaiter`：实现`awaitable`，在`await_suspend`被调用时，向`Scheduler`注册当前暂停的协程以及一个用于检查是否可恢复的算子。比如通过`DelayAwaiter`的实现可以更容易理解：

```cpp
    void await_suspend(std::coroutine_handle<> handle) noexcept {
        auto now = std::chrono::system_clock::now();

        Scheduler::instance().setupCoro(handle, [t = this->_timeout, start = now](){
            return std::chrono::system_clock::now() - start >= t;
        });
    }
```

当超时后，调度器查询可得知这个协程可恢复，并由调度器负责恢复。

`Channel`：指定容量上限的队列，可用于协程间通信。

## 意义

“手动”循环调用`Scheduler::dispatch()`来调度的好处是可以把协程调度嵌入到各种框架中。比如UI的事件循环，在一个时钟或UI空闲时调度协程，可以使协程与UI在同一个线程内。

很多在操作系统层面的异步操作并没有提供回调函数，而是通过改变信号锁等方式来通知异步操作完成。这时本协程库的“轮询”逻辑就可以很自然的应用上去。就像`FutureAwaiter`的实现：

```cpp
    void await_suspend(std::coroutine_handle<> handle) noexcept {
        Scheduler::instance().setupCoro(
            handle,
            [v = this->_value] () {
                return v.wait_for(std::chrono::milliseconds::zero()) == std::future_status::ready;
            }
        );
    }
```

