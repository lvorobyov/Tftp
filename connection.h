//
// Created by Lev on 13.02.2019.
//

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
