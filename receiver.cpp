//
// Created by Lev on 13.02.2019.
//

#include <stdexcept>
#include <iostream>
#include <ctime>
#include <list>

#include "socket.h"
#include "receiver.h"
#include "writer.h"

#include <mingw.thread.h>
using namespace std;

DWORD tftp::receiver::thread_main() noexcept {
    // Listen TCP clients
    list<connection> connections;
    fiber_primary primary;
    writer<list> assistant(connections, primary);
    auto cores = std::thread::hardware_concurrency();
    try {
        if (cores > 1)
            assistant.start();
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
        fd_set fds, sss{};
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        TIMEVAL timeout{0,500};
        do {
            sss = fds;
            int s = select(0, &sss, nullptr, nullptr, &timeout);
            if (s == SOCKET_ERROR)
                throw logic_error("select error");
            if (s == 0)
                continue;
            if (FD_ISSET(sock, &sss)) {
                sockaddr_in addr{ PF_INET };
                int addr_len = sizeof(addr);
                SOCKET client = accept(sock, (sockaddr*)&addr, &addr_len);
                if (client == INVALID_SOCKET)
                    throw logic_error("accept failed");
                ioctlsocket(client, FIONBIO, &mode);
                connections.emplace_back(client,addr.sin_addr,primary);
                FD_SET(client, &fds);
                s --;
            }
            auto it = connections.begin();
            while (s > 0 && it != connections.end()) {
                if (FD_ISSET(it->get_sock(), &sss)) {
                    s --;
                    it->yield_from(primary);
                    it->set_written();
                    if (! it->is_active()) {
                        FD_CLR(it->get_sock(), &fds);
                        it = connections.erase(it);
                        continue;
                    }
                    if (! it->is_writing())
                        it->get_received().set();
                }
                it++;
            }
        } while (active);
    } catch (logic_error const& ex) {
        cerr << ex.what() << " code " << WSAGetLastError() << endl;
    }
    if (cores > 1)
        assistant.stop();
    return 0;
}

tftp::receiver::~receiver() {
    closesocket(sock);
}

void tftp::receiver::stop() noexcept {
    active = false;
}
