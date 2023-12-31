#ifndef MYCORO_AWAITERS_H
#define MYCORO_AWAITERS_H

#include "scheduler.h"
#include <chrono>

namespace mycoro{

template< class Rep, class Period >
struct DelayAwaiter
{
    explicit DelayAwaiter(const std::chrono::duration<Rep,Period>& timeout_duration)
    :_timeout(timeout_duration)
    {
    }

    bool await_ready() const noexcept {
        return _timeout.count() <= 0;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        auto now = std::chrono::system_clock::now();

        Scheduler::instance().setupCoro(handle, [t = this->_timeout, start = now](){
            return std::chrono::system_clock::now() - start >= t;
        });
    }

    void await_resume() noexcept {}
private:
    std::chrono::duration<Rep,Period> _timeout;
};

template< class Rep, class Period >
using delay = DelayAwaiter<Rep, Period>;

struct AwakeAwaiter
{
    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        Scheduler::instance().setupCoro(handle, [](){ return true; });
    }

    void await_resume() noexcept {}
};


}

#endif