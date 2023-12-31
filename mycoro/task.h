#ifndef MYCORO_TASK_H
#define MYCORO_TASK_H

#include <optional>
#include <coroutine>
#include "result.h"
#include "scheduler.h"
#include "awaiters.h"
namespace mycoro {

template<typename T>
struct TaskAwaiter;

template<typename T>
struct Task
{
    using value_type = T;

    struct promise_type
    {
        AwakeAwaiter initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { 
            std::cout << "final_suspend" << std::endl;
            return {}; }
        void unhandled_exception() { _result = Result<value_type>(std::current_exception()); }
        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        void return_value(value_type v) { _result = Result<value_type>{std::move(v)}; }
        const value_type& get_result()
        {
            return _result->get_or_throw();
        }
        
        template <typename T2>
        TaskAwaiter<T2> await_transform(Task<T2> &&task)
        {
            return TaskAwaiter<T2>(std::move(task));
        }

        template <typename T2>
        decltype(auto) await_transform(T2&& awaitor)
        {
            return std::forward<T2>(awaitor);
        }

    private:
        std::optional<Result<value_type>> _result{};
    };

    explicit Task(std::coroutine_handle<promise_type> handle)
        :_handle(handle)
    {}

    bool done() const noexcept
    {
        return _handle.done();
    }

    const value_type& get_result()
    {
        return _handle.promise().get_result();
    }

private:
    std::coroutine_handle<promise_type> _handle;
};

template<>
struct Task<void>
{
    struct promise_type
    {
        AwakeAwaiter initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { 
            std::cout << "final_suspend" << std::endl;
            return {}; }
        void unhandled_exception() { _result = Result<int>(std::current_exception()); }
        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        void return_void() { _result = Result<int>{0}; }
        void get_result()
        {
            _result->get_or_throw();
        }
        
        template <typename T2>
        TaskAwaiter<T2> await_transform(Task<T2> &&task)
        {
            return TaskAwaiter<T2>(std::move(task));
        }

        template <typename T2>
        decltype(auto) await_transform(T2&& awaitor)
        {
            return std::forward<T2>(awaitor);
        }

    private:
        std::optional<Result<int>> _result{};
    };

    explicit Task(std::coroutine_handle<promise_type> handle)
        :_handle(handle)
    {}

    bool done() const noexcept
    {
        return _handle.done();
    }

    void get_result()
    {
        _handle.promise().get_result();
    }

private:
    std::coroutine_handle<promise_type> _handle;
};

template<typename T>
struct TaskAwaiter
{
    using value_type = T;

    explicit TaskAwaiter(Task<value_type> &&task)
    :_task(std::move(task)){}

    TaskAwaiter(TaskAwaiter&) = delete;
    TaskAwaiter& operator=(const TaskAwaiter&) = delete;

    constexpr bool await_ready() const noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        Scheduler::instance().setupCoro(handle, [t = this->_task](){ return t.done(); });
    }

    value_type await_resume() noexcept {
        return _task.get_result();
    }
private:
    Task<value_type> _task;
};

}

#endif