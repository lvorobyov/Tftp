// Copyright (c) 2021, Лев Воробьёв
// This file is licensed under GPL v3

#ifndef TFTP_FILE_H
#define TFTP_FILE_H

#include <cstdio>

namespace tftp {
    using file_t = std::unique_ptr<FILE, decltype(&fclose)>;
}

#endif //TFTP_FILE_H
