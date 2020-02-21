#pragma once

#include <iostream>

namespace p2p {

    template <typename F>
    struct deferred_op
    {
        template<typename T>
        deferred_op(T&& f)
            : _f(std::forward<T>(f)), _moved(false)
        {}

        deferred_op(deferred_op&& other)
            : _f(std::move(other._f)), _moved(other._moved)
        {
            other.m_isMoved = true;
        }

        ~deferred_op()
        {
            if (!_moved) _f();
        }

    private:
        F    _f;
        bool _moved;
    };

    template <typename F>
    deferred_op<F> defer_impl(F&& f)
    {
        return deferred_op<F>(std::forward<F>(f));
    }

    auto log_scope_impl(const std::string& scope_name)
    {
        std::cout << "Enter " << scope_name << std::endl;
        return defer_impl([scope_name] {std::cout << "Exits " << scope_name << std::endl; });
    }

#define defer(x) auto defer_##__COUNTER__ = defer_impl(x);

#define log_scope auto log_scope_##__COUNTER__ = log_scope_impl(__func__);
#define log_scope_(x) auto log_scope_##__COUNTER__ = log_scope_impl(x);

}