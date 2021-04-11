// Copyright (c) 2019, 2021, Лев Воробьёв
// This file is licensed under GPL v3

#include <stdexcept>
#include <spdlog/spdlog.h>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

#include "socket.h"
#include "receiver.h"

using namespace tftp;

using std::logic_error;
using boost::asio::thread_pool;
using boost::asio::post;

receiver::receiver(volatile sig_atomic_t &active) //: _worker([](receiver *self)
{
    // Listen TCP clients
    try {
        socket_t sock (socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), &closesocket);
        if (sock == INVALID_SOCKET)
            throw logic_error("socket fail");
        sockaddr_in serv{PF_INET};
        serv.sin_port = htons(LISTEN_PORT);
        serv.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sock, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw logic_error("bind failed");
        if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
            throw logic_error("listen error");
#ifdef _WIN32
        u_long mode = 0;
        if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR)
            throw logic_error("ioctlsocket error");
#else
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == SOCKET_ERROR)
            throw logic_error("fcntl 1 error");
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == SOCKET_ERROR)
            throw logic_error("fcntl 2 error");
#endif
        thread_pool pool(thread::hardware_concurrency());
        fd_set fds;
        do {
            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            timeval timeout{0,500};
            int s = select(0, &fds, nullptr, nullptr, &timeout);
            if (s == SOCKET_ERROR)
                throw logic_error("select error");
            if (s == 0)
                continue;
            sockaddr_in addr{ PF_INET };
            socklen_t addr_len = sizeof(addr);
            SOCKET client = accept(sock, (sockaddr*)&addr, &addr_len);
            if (client == INVALID_SOCKET)
                throw logic_error("accept failed");
            spdlog::info("Transmit file from {}", inet_ntoa(addr.sin_addr));
            post(pool, connection(client));
        } while (active);
        pool.join();
    } catch (std::exception const& ex) {
        spdlog::error("{} (code {})", ex.what(), errno);
    }
} //, this) { }

