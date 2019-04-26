//
// Created by Lev on 13.02.2019.
//

#ifndef TFTP_CONNECTION_H
#define TFTP_CONNECTION_H

#include "socket.h"
#include <win32/fiber.h>
#include <win32/event.h>
#include <win32/mutex.h>

namespace tftp {

    using namespace csoi::win32;

    class connection : public fiber {
    protected:
        SOCKET sock;
        in_addr addr;
        fiber_primary &owner;
        fiber_primary *auxiliary = nullptr;
        bool active = true;
        event received{false};
        mutex guard;
        bool writing = false;
        event written;

    public:
        explicit connection(SOCKET sock, in_addr addr, fiber_primary &owner);

        SOCKET get_sock() const;

        const in_addr &get_addr() const;

        bool is_active() const;

        bool is_writing() const;

        void set_writing(bool writing);

        void set_auxiliary(fiber_primary &auxiliary_fiber);

        const event &get_received() const;

        const mutex &get_guard() const;

        void yield_from(const fiber_primary &primary);

        ~connection() override;

    protected:
        void fiber_main() noexcept override;
    };

}


#endif //TFTP_CONNECTION_H
