//
// Created by Lev on 13.02.2019.
//

#include "connection.h"
#include "file.h"
#include <ctime>

#define BUFFER_SIZE 512

using namespace tftp;

connection::connection(SOCKET sock) : sock(sock, &closesocket) {}

void connection::operator()() noexcept {
    // Download file
    time_t t = time(nullptr);
    char filename[MAX_PATH];
    sprintf(filename, "downloaded_%li%d.mp4", t, rand());
    file_t f(fopen(filename, "wb+"), &fclose);
    char buf[BUFFER_SIZE];
    ssize_t len;
    while((len = recv(sock, buf, BUFFER_SIZE, 0)) > 0) {
        fwrite(buf, sizeof(char), static_cast<size_t>(len), f.get());
    }
    shutdown(sock, SHUT_WR);
}

