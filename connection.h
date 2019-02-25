//
// Created by Lev on 13.02.2019.
//

#ifndef TFTP_CONNECTION_H
#define TFTP_CONNECTION_H

#include "socket.h"
#include <win32/thread.h>

namespace tftp {

    using namespace csoi::win32;

    class connection : public thread<connection> {
    protected:
        SOCKET sock;
        in_addr addr;

    public:
        explicit connection(SOCKET sock, in_addr addr);

        const in_addr &get_addr() const;

        ~connection() override;

    protected:
        DWORD thread_main() noexcept override;
    };

}


#endif //TFTP_CONNECTION_H
