#ifndef MYCORO_SCHEDULER_H
#define MYCORO_SCHEDULER_H

#include <functional>
#include <vector>
#include <coroutine>
#include <algorithm>

namespace mycoro
{
    class Scheduler
    {
    public:
        size_t size() const
        {
            return _coroList.size();
        }

        void dispatch()
        {
            for(auto itr = _coroList.begin(); itr!= _coroList.end(); ++itr)
            {
                auto& h = *itr;
                if(h.check_ready())
                {
                    // check_ready为true后，这个函数不再使用
                    h.check_ready = [](){return false;};

                    h.handle();
                    
                    if(h.handle.done())
                    {
                        // h.handle.destroy();
                        _coroList.erase(itr);
                        std::cout << "handle done, removed from list" << std::endl;
                    }
                    break;
                }
            }
        }

        void setupCoro(std::coroutine_handle<> handle, std::function<bool()> check_ready)
        {
            auto itrExists = std::find_if(_coroList.begin(), _coroList.end(), [addr = handle.address()](const CoroHandle& h){
                return h.handle.address() == addr;
            });
            if(itrExists != _coroList.end())
            {
                itrExists->check_ready = std::move(check_ready);
            }
            else
            {
                _coroList.emplace_back(CoroHandle{.handle = handle, .check_ready = std::move(check_ready)});
            }
        }

        bool has(std::coroutine_handle<> handle) const
        {
            return std::find_if(_coroList.begin(), _coroList.end(), [addr = handle.address()](const CoroHandle& h){
                return h.handle.address() == addr;
            }) != _coroList.end();
        }

        static Scheduler& instance()
        {
            static Scheduler sch;
            return sch;
        }

        Scheduler(const Scheduler&) = delete;
        Scheduler& operator=(const Scheduler&) = delete;
    private:
        struct CoroHandle
        {
            std::coroutine_handle<> handle;
            std::function<bool()> check_ready;
        };

        std::vector<CoroHandle> _coroList;

        Scheduler() = default;
    };
}

#endif