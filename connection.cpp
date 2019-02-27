//
// Created by Lev on 13.02.2019.
//

#include "connection.h"
#include <cstdio>
#include <ctime>

#define BUFFER_SIZE 2048

tftp::connection::connection(SOCKET sock, in_addr addr) : sock(sock), addr(addr) {}

DWORD tftp::connection::thread_main() noexcept {
    // Download file
    time_t t = time(nullptr);
    char filename[MAX_PATH];
    sprintf_s(filename, MAX_PATH, "downloaded_%d.mp4", t);
    FILE *f = fopen(filename, "wb+");
    char buf[BUFFER_SIZE];
    int len;
    while((len = recv(sock, buf, BUFFER_SIZE, 0)) != 0) {
        fwrite(buf, sizeof(char), static_cast<size_t>(len), f);
    }
    fclose(f);
    return 0;
}

tftp::connection::~connection() {
    closesocket(sock);
}

const in_addr &tftp::connection::get_addr() const {
    return addr;
}
