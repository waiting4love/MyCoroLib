#ifndef MYCORO_CHAN_H
#define MYCORO_CHAN_H

#include "scheduler.h"
#include <queue>

namespace mycoro {

template<typename T>
struct Channel
{
    using value_type = T;
    
    struct PutAwaiter
    {
        PutAwaiter(Channel& ch, value_type v)
        :_ch(ch), _v(std::move(v))
        {
        }

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) noexcept {
            Scheduler::instance().setupCoro(handle, [this](){ 
                if(_ch._queue.size() < _ch._cap)
                {
                    _ch._queue.push(std::move(_v));
                    return true;
                }
                return false;
            });
        }

        void await_resume() noexcept {}
    private:
        Channel& _ch;
        value_type _v;
    };

    struct GetAwaiter
    {
        explicit GetAwaiter(Channel& ch)
        :_ch(ch)
        {
        }

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> handle) noexcept {
            Scheduler::instance().setupCoro(handle, [this](){ 
                if(!_ch._queue.empty())
                {
                    _v = _ch._queue.front();
                    _ch._queue.pop();
                    return true;
                }
                return false;
            });
        }

        const value_type& await_resume() noexcept {
            return _v;
        }
    private:
        Channel& _ch;
        value_type _v;
    };

    explicit Channel(int capable)
    :_cap(capable)
    {
    }

    PutAwaiter put(value_type v)
    {
        return PutAwaiter(*this, std::move(v));
    }

    GetAwaiter get()
    {
        return GetAwaiter(*this);
    }

private:
    std::queue<value_type> _queue;
    int _cap;
};

}

#endif