//
// Created by Lev on 13.02.2019.
//

#include "connection.h"
#include <cstdio>
#include <ctime>
#include <random>

#define BUFFER_SIZE 512

tftp::connection::connection(SOCKET sock, in_addr addr, fiber_primary &owner) : sock(sock), addr(addr), owner(owner) {}

void tftp::connection::fiber_main() noexcept {
    // Download file
    time_t t = time(nullptr);
    static random_device rd;
    static mt19937 mt(t + rd());
    static uniform_int_distribution uid(1000,9999);
    char filename[MAX_PATH];
    sprintf_s(filename, MAX_PATH, "downloaded_%d_%d.mp4", t, uid(mt));
    FILE *f = fopen(filename, "wb+");
    char buf[BUFFER_SIZE];
    int len;
    while((len = recv(sock, buf, BUFFER_SIZE, 0)) != 0) {
        if (auxiliary)
            switch_to(owner);
        writing = true;
        fwrite(buf, sizeof(char), static_cast<size_t>(len), f);
        switch_to(*(auxiliary ?: &owner));
    }
    fclose(f);
    active = false;
    switch_to(owner);
}

tftp::connection::~connection() {
    closesocket(sock);
}

const in_addr &tftp::connection::get_addr() const {
    return addr;
}

bool tftp::connection::is_active() const {
    return active;
}

SOCKET tftp::connection::get_sock() const {
    return sock;
}

const csoi::win32::event &tftp::connection::get_received() const {
    return received;
}

void tftp::connection::set_auxiliary(csoi::win32::fiber_primary &auxiliary_fiber) {
    this->auxiliary = &auxiliary_fiber;
}

const csoi::win32::mutex &tftp::connection::get_guard() const {
    return guard;
}

void tftp::connection::yield_from(const fiber_primary &primary) {
    guard.wait();
    primary.switch_to(*this);
    guard.release();
}

bool tftp::connection::is_writing() const {
    return writing;
}

void tftp::connection::set_writing(bool writing) {
    connection::writing = writing;
}
