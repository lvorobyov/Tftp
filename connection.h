// Copyright (c) 2019, 2021, Лев Воробьёв
// This file is licensed under GPL v3

#ifndef TFTP_CONNECTION_H
#define TFTP_CONNECTION_H

#include "socket.h"

namespace tftp {
    class connection {
    protected:
        socket_t sock;

    public:
        explicit connection(SOCKET sock);

        void operator()() noexcept;
    };

}


#endif //TFTP_CONNECTION_H
