//
// Created by Lev on 13.02.2019.
//

#include <stdexcept>
#include <iostream>
#include <memory>
#include <ctime>
#include <cstdio>

#include "socket.h"
#include "receiver.h"
#include "scheduler.h"

using namespace std;

DWORD tftp::receiver::thread_main() noexcept {
    // Listen TCP clients
    FILE *log = fopen("tftps.log", "a+");
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
        u_long mode = 0;
        ioctlsocket(sock, FIONBIO, &mode);
        fd_set fds;
        time_t tm;
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
            char *str_time = asctime(localtime(&tm));
            str_time[strlen(str_time)-1] = '\0';
            fprintf(log, "%s accepted %s\n", str_time, inet_ntoa(addr.sin_addr));
            auto conn = make_shared<connection>(client,addr.sin_addr);
            connections.execute(conn);
        } while (active);
        connections.shutdown();
        connections.wait();
    } catch (logic_error const& ex) {
        cerr << ex.what() << " code " << WSAGetLastError() << endl;
    }
    fclose(log);
    return 0;
}

tftp::receiver::~receiver() {
    closesocket(sock);
}

void tftp::receiver::stop() noexcept {
    active = false;
}
