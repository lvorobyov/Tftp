//
// Created by Lev on 21.02.2019.
//

#ifndef TFTP_DETECTOR_H
#define TFTP_DETECTOR_H


#include <cstdint>

namespace tftp {

    class detector {
    public:
        detector() = delete;

        const char* detect(uint8_t bytes);
    };

}


#endif //TFTP_DETECTOR_H
