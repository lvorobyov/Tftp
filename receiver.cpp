//
// Created by Lev on 13.02.2019.
//

#include <stdexcept>
#include <iostream>
#include <memory>
#include <ctime>
#include <plog/Log.h>

#include "socket.h"
#include "receiver.h"
#include "scheduler.h"

using namespace std;

DWORD tftp::receiver::thread_main() noexcept {
    // Listen TCP clients
    scheduler<connection> connections;
    connections.start();
    try {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
            throw logic_error("socket fail");
        sockaddr_in serv{PF_INET};
        serv.sin_port = htons(LISTEN_PORT);
        serv.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sock, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR)
            throw logic_error("bind failed");
        if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
            throw logic_error("listen error");
        LOGD << "listen started";
        u_long mode = 0;
        ioctlsocket(sock, FIONBIO, &mode);
        fd_set fds;
		time_t tm;
		char str_addr[INET_ADDRSTRLEN];
        do {
            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            TIMEVAL timeout{0,500};
            int s = select(0, &fds, nullptr, nullptr, &timeout);
            if (s == SOCKET_ERROR)
                throw logic_error("select error");
            if (s == 0)
                continue;
            sockaddr_in addr{ PF_INET };
            int addr_len = sizeof(addr);
            SOCKET client = accept(sock, (sockaddr*)&addr, &addr_len);
            if (client == INVALID_SOCKET)
                throw logic_error("accept failed");
			time(&tm);
            inet_ntop(AF_INET, &addr.sin_addr, str_addr, INET_ADDRSTRLEN);
			LOG_INFO << "accepted " << str_addr;
            auto conn = make_shared<connection>(client,addr.sin_addr);
            connections.execute(conn);
        } while (active);
        connections.shutdown();
        connections.wait();
    } catch (logic_error const& ex) {
        cerr << ex.what() << " code " << WSAGetLastError() << endl;
    }
    return 0;
}

tftp::receiver::~receiver() {
    LOGD << "listen closed";
    closesocket(sock);
}

void tftp::receiver::stop() noexcept {
    active = false;
}
