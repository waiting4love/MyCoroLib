#ifndef MYCORO_RESULT_H
#define MYCORO_RESULT_H

#include <exception>

namespace mycoro
{

    template <typename T>
    struct Result
    {
        Result() = default;

        explicit Result(T value) : _value(std::move(value)) {}

        explicit Result(std::exception_ptr &&exception_ptr) : _exception_ptr(exception_ptr) {}

        const T& get_or_throw() const
        {
            if (_exception_ptr)
            {
                std::rethrow_exception(_exception_ptr);
            }
            return _value;
        }

    private:
        T _value{};
        std::exception_ptr _exception_ptr;
    };

}
#endif