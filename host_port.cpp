// Copyright (c) 2020, Лев Воробьёв
// This file is licensed under GPL v3

#include <cstring>
#include <stdexcept>
#include <cassert>
#include "host_port.h"

using namespace utils;

host_port::host_port(const char *str) {
    char * host = strdup(str);
    // номера порта доолжен быть
    char *p = strchr(host, ':');
    if (p != nullptr) {
        *p++ = '\0';
        _host = host;
        assert(*(p - 1) != ':');
    } else {
        p = host;
    }
    // после двоеточия должно быть число, и ничего более
    u_long l = strtoul(p, &p, 10);
    if (l == 0L || *p != '\0' || l > 65535)
        throw invalid_argument("port");
    // число после двоеточия не должно превышать 65535
    _port = static_cast<u_short>(l);
    free(host);
}

const string &host_port::get_host() const {
    return _host;
}

u_short host_port::get_port() const {
    return _port;
}
