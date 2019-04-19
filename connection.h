//
// Created by Lev on 13.02.2019.
//

#ifndef TFTP_CONNECTION_H
#define TFTP_CONNECTION_H

#include "socket.h"
#include <win32/fiber.h>

namespace tftp {

    using namespace csoi::win32;

    class connection : public fiber {
    protected:
        SOCKET sock;
        in_addr addr;
        fiber_primary &owner;
        bool active = true;

    public:
        explicit connection(SOCKET sock, in_addr addr, fiber_primary &owner);

        SOCKET get_sock() const;

        const in_addr &get_addr() const;

        bool is_active() const;

        ~connection() override;

    protected:
        void fiber_main() noexcept override;
    };

}


#endif //TFTP_CONNECTION_H
