//
// Created by Lev on 13.02.2019.
//

#ifndef TFTP_RECEIVER_H
#define TFTP_RECEIVER_H

#define LISTEN_PORT 8969

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
