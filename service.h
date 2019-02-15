//
// Created by Lev on 15.02.2019.
//

#ifndef TFTP_SERVICE_H
#define TFTP_SERVICE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tftp {

    class service {
    private:
        SERVICE_STATUS_HANDLE status_handle;
        SERVICE_STATUS status;
        DWORD check_point;

    public:

    };

}


#endif //TFTP_SERVICE_H
