// Copyright (c) 2021, Лев Воробьёв
// This file is licensed under GPL v3

#ifndef TFTP_HELPER_H
#define TFTP_HELPER_H

#include <memory>
#include <functional>
#include <utility>

namespace tftp {

    template <typename T, T default_value>
    class nullable {
        T _val;

    public:
        nullable() : _val(default_value) {}
        nullable(T val) : _val(val) {}
        nullable(nullptr_t) : _val(default_value) {}

        operator T() { return _val; }

        bool operator ==(const nullable<T,default_value> &other) const {return _val == other._val;}
        bool operator !=(const nullable<T,default_value> &other) const {return _val != other._val;}
        bool operator ==(std::nullptr_t) const {return _val == default_value;}
        bool operator !=(std::nullptr_t) const {return _val != default_value;}

        operator const bool() const {return _val != default_value;}

        using element = T;
    };

    template <typename T, typename F>
    class deleter {
        F _f;

    public:
        explicit deleter(F && f) : _f(std::forward<F>(f)) {}

        void operator()(T val) {
            _f(val);
        }

        using pointer = T;
    };

    template <typename T, T default_value, typename F>
    class unique {
        std::unique_ptr<T, deleter<nullable<T,default_value>,F>> ptr;

    public:
        unique(T val, F && f) : ptr(val, deleter<nullable<T,default_value>,F>(std::forward<F>(f))) { }
        unique &operator=(T val) {
            ptr.reset(val);
            return *this;
        }
        operator T() {
            return ptr.get();
        }
        template <typename U>
        bool operator==(U val) {
            return static_cast<T>(*this) == val;
        }
        template <typename U>
        bool operator!=(U val) {
            return static_cast<T>(*this) != val;
        }
    };

}

#endif //TFTP_HELPER_H
