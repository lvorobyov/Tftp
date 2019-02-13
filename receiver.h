//
// Created by Lev on 13.02.2019.
//

#ifndef TFTP_RECEIVER_H
#define TFTP_RECEIVER_H

#define LISTEN_PORT 8969

#include <win32/thread.h>

namespace tftp {

    using namespace csoi::win32;

    class receiver : public thread<receiver> {
    private:
        SOCKET sock = INVALID_SOCKET;

    public:
        ~receiver() override;

        void stop() noexcept;

    protected:
        DWORD thread_main() noexcept override;

        bool active = true;
    };

}


#endif //TFTP_RECEIVER_H
