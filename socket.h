// Copyright (c) 2019, 2021, Лев Воробьёв
// This file is licensed under GPL v3

#ifndef TFTP_SOCKET_H
#define TFTP_SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define errno WSAGetLastError()
#define ioctl ioctlsocket
#define EAGAIN WSAETIMEDOUT
#define SHUT_WR SD_SEND
typedef WORD uint16_t;
typedef DWORD uint32_t;
typedef int socklen_t;
typedef TIMEVAL timeval;
typedef int ssize_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h> // close
#include <fcntl.h>  // fcntl
#include <cerrno>
typedef int SOCKET;
#define closesocket close
#define ioctlsocket fcntl
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
typedef int BOOL;
#define TRUE (1)
#define FALSE (0)
#define MAX_PATH (80)
#endif

#include "helper.h"

namespace tftp {
    using socket_t = unique<SOCKET, INVALID_SOCKET, decltype(&closesocket)>;

#ifdef _WIN32
    class wsa_data {
        WSADATA wsd{0};
    public:
        wsa_data() noexcept {
            if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
                exit(EXIT_FAILURE);
        }

        ~wsa_data() noexcept {
            WSACleanup();
        }

        wsa_data(wsa_data const&) = delete;
        wsa_data(wsa_data &&) = delete;

        wsa_data &operator=(wsa_data const&) = delete;
        wsa_data &operator=(wsa_data &&) = delete;
    };
#endif

    inline int set_timeout(socket_t &sock, int option, uint32_t timeout) {
#ifdef _WIN32
        DWORD tv = timeout;
#else
        timeval tv = {timeout / 1000, (timeout % 1000) * 1000};
#endif
        return setsockopt(sock, SOL_SOCKET, option, (char*)&tv, sizeof(tv));
    }

    inline int set_option(socket_t &sock, int option, bool value) {
        BOOL val = value? TRUE : FALSE;
        return setsockopt(sock, SOL_SOCKET, option, (char*)&val, sizeof(val));
    }
}

#endif //TFTP_SOCKET_H
