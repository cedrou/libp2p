#pragma once

#include <iostream>

namespace p2p {

    class exceptor
    {
    public:
        class exception : public std::exception
        {
        public:
            exception(std::error_code error) : std::exception(error.message().c_str()), error(error) {}
            std::error_code error;
        };

    public:
        exceptor() : _read(false)
        {}
        exceptor(std::error_code&& error)
            : _read(false), _error(error)
        { }

        exceptor(exceptor& e)
            : _read(e._read), _error(e._error)
        {
            e._read = true;
        }

        exceptor(exceptor&& e)
            : _read(false), _error(e._error)
        {
            e._read = true;
        }

        exceptor& operator=(exceptor& e)
        {
            throw_if_not_read();

            _error = e._error;
            _read = e._read;
            e._read = true;
            return *this;
        }

        exceptor& operator=(exceptor&& e)
        {
            throw_if_not_read();

            _error = e._error;
            _read = false;
            e._read = true;
            return *this;
        }

        ~exceptor() noexcept(false)
        {
            throw_if_not_read();
        }

        operator bool()
        {
            _read = true;
            return static_cast<bool>(_error);
        }

    private: // prevent "new" allocation
        inline void *operator new(size_t) { return nullptr; }
        inline void operator delete(void*) {}
        inline void *operator new[](size_t) { return nullptr; }
            inline void operator delete[](void*) {}

            void throw_if_not_read()
        {
            if (!_read && _error)
            {
                throw exception(_error);
            }
        }

    private:
        bool _read;
        std::error_code _error;
    };

}