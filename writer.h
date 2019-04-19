//
// Created by Lev on 19.04.2019.
//

#ifndef _WRITER_H_
#define _WRITER_H_

#include <win32/thread.h>
#include "connection.h"

namespace tftp {

    using namespace csoi::win32;

    class writer : public thread<writer> {
    protected:
        DWORD thread_main() noexcept override;
    };

}

#endif //_WRITER_H_
