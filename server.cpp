//
// Created by Lev on 12.02.2019.
//

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
using std::logic_error;

#include "socket.h"
#include "receiver.h"
using namespace tftp;

#define BUFFER_SIZE 512

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

void start_server();

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %T:%f %z] [%^%=8l%$] [thread %t] %v");
    spdlog::stdout_color_st("stdout_log", spdlog::color_mode::automatic);
    spdlog::set_default_logger(spdlog::get("stdout_log"));
#ifdef _WIN32
    wsa_data wsa;
#endif
	try {
		start_server();
	} catch(std::exception const& ex) {
		printf("%s: %d\n", ex.what(), errno);
		return EXIT_FAILURE;
	}
    printf("Bye!\n");
    return EXIT_SUCCESS;
}

void start_server() {
    static volatile sig_atomic_t active = true;
    signal(SIGINT, [](int s) { active = false; });
    thread([]() {
        try {
            char buf[BUFFER_SIZE];
            sockaddr_in addr{AF_INET};
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(LISTEN_PORT);
            socket_t sock(socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP), &closesocket);
            if (sock == INVALID_SOCKET)
                throw logic_error("socket error");
            if (bind(sock, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
                throw logic_error("bind failed");
            if (set_option(sock, SO_BROADCAST, true) == SOCKET_ERROR)
                throw logic_error("option setting error");
            if (set_timeout(sock, SO_RCVTIMEO, 500) == SOCKET_ERROR)
                throw logic_error("timeout setting error");
            sockaddr_in client{};
            socklen_t length = sizeof(client);
            do {
                ssize_t n = recvfrom(sock, buf, BUFFER_SIZE, 0,
                                     (sockaddr *) &client, &length);
                if (n == SOCKET_ERROR) {
                    if (errno != EAGAIN)
                        throw logic_error("receive failed");
                    else continue;
                }
                char hostname[HOST_NAME_MAX];
                gethostname(hostname, HOST_NAME_MAX);
                if (sendto(sock, hostname, strlen(hostname), 0, (sockaddr *) &client, length) == SOCKET_ERROR)
                    throw logic_error("send failed");
            } while (active);
        } catch (std::exception const& ex) {
            spdlog::critical(ex.what());
        }
    }).detach();
    receiver thread1(active);
}