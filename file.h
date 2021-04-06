//
// Created by Lev on 06.04.2021.
//

#ifndef TFTP_FILE_H
#define TFTP_FILE_H

#include <cstdio>

namespace tftp {
    using file_t = std::unique_ptr<FILE, decltype(&fclose)>;
}

#endif //TFTP_FILE_H
