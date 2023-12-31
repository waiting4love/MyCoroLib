#ifndef MYCORO_ASYNC_H
#define MYCORO_ASYNC_H

#include "scheduler.h"
#include <future>

namespace mycoro {

template<typename T>
struct FutureAwaiter
{
    using value_type = T;

    explicit FutureAwaiter(std::shared_future<value_type> f)
    :_value(f){}

    bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        Scheduler::instance().setupCoro(
            handle,
            [v = this->_value] () {
                return v.wait_for(std::chrono::milliseconds::zero()) == std::future_status::ready;
            }
        );
    }

    value_type await_resume() noexcept {
        return _value.get();
    }
private:
    std::shared_future<value_type> _value;
};

template<typename Fn, typename... Args>
struct AsyncAwaiter
{
    using value_type = std::result_of_t<Fn(Args&&...)>;
    
    explicit AsyncAwaiter(Fn&& fn, Args&&... args)
    {
        _value = std::async(std::launch::async, std::forward<Fn>(fn), std::forward<Args>(args)...);
    }

    FutureAwaiter<value_type> operator co_await() const
    {
        return FutureAwaiter<value_type>(_value);
    }
private:
    std::shared_future<value_type> _value;
};

template<typename Fn, typename... Args>
using async = AsyncAwaiter<Fn, Args...>;

}
#endif