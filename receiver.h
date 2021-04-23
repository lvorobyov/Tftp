// Copyright (c) 2019, 2021, Лев Воробьёв
// This file is licensed under GPL v3

#ifndef TFTP_RECEIVER_H
#define TFTP_RECEIVER_H

#include <thread>

#include "connection.h"

namespace tftp {
    using std::thread;

    class receiver {
    private:
//        thread _worker;
//        volatile bool active = true;

    public:
        explicit receiver(volatile sig_atomic_t &active);
    };

}


#endif //TFTP_RECEIVER_H
