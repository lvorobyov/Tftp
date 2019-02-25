//
// Created by Lev on 13.02.2019.
//

#include "socket.h"
#include "receiver.h"
#include <stdexcept>
#include <iostream>
#include <map>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace std::chrono;

#define TIME_TOLERANCE milliseconds(1000)

DWORD tftp::receiver::thread_main() noexcept {
    // Listen TCP clients
    vector<shared_ptr<connection>> connections;
    map<u_long, time_point<steady_clock>> timetable;
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
        u_long mode = 0;
        ioctlsocket(sock, FIONBIO, &mode);
        fd_set fds;
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
            auto tm = steady_clock::now();
            auto key = addr.sin_addr.s_addr;
            auto ls = timetable.find(key);
            if (ls != timetable.end() && tm < ls->second + TIME_TOLERANCE && !accomplish(connections, key)) {
                closesocket(client);
                continue;
            }
            timetable[key] = tm;
            auto conn = make_shared<connection>(client,addr.sin_addr);
            conn->start();
            connections.push_back(conn);
        } while (active);
    } catch (logic_error const& ex) {
        cerr << ex.what() << " code " << WSAGetLastError() << endl;
    }
    for (auto& c: connections)
	    c->wait();
    return 0;
}

tftp::receiver::~receiver() {
    closesocket(sock);
}

void tftp::receiver::stop() noexcept {
    active = false;
}

bool tftp::receiver::accomplish(const vector<shared_ptr<connection>> &connections, u_long key) {
    for (const auto &conn: connections) {
        if (conn->get_addr().s_addr == key) {
            if (! conn->wait(0))
                return false;
        }
    }
    return true;
}
