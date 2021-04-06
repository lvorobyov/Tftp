#include <cstring>
#include <thread>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "socket.h"
#include "file.h"
using namespace tftp;

using std::logic_error;
using std::cout;
using std::endl;
using std::vector;
using std::string;

#include <cxxopts.hpp>
using namespace cxxopts;

#include "host_port.h"
using btc::host_port;

#define TFTP_PORT ((uint16_t)8969)

#define BUFFER_SIZE 512

#define BROADCAST "Broadcast"

void discover(uint32_t timeout, uint16_t port, vector<sockaddr_in> &peers);

void transfer(const sockaddr_in &peer, const char* filename);

int main(int argc, char* argv[]) {
#ifdef _WIN32
    wsa_data wsa;
#endif
    Options options(argv[0], "Transfer file client 1.0");
    options.add_options()
            ("o,host", "receiver", value<string>())
            ("t,timeout", "discover delay timeout", value<uint32_t>())
            ("h,help", "show this help");
    auto result = options.parse(argc,argv);
    if (result.count("help")) {
        cout << options.help() << endl;
        return 0;
    }
    uint32_t timeout = result.count("t")? result["t"].as<uint32_t>() : 500;
    sockaddr_in peer{0};
    int status = EXIT_SUCCESS;
    try {
        string host;
        uint16_t port;
        if (result.count("host")) {
            host_port hp(result["host"].as<string>().c_str());
            host = hp.get_host();
            port = hp.get_port();
        } else {
            port = TFTP_PORT;
        }
        if (! host.empty()) {
            addrinfo hints{0};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            addrinfo *pai;
            char str_port[6];
            sprintf(str_port, "%d", port);
            if (getaddrinfo(host.c_str(), str_port, &hints, &pai) == SOCKET_ERROR)
                throw logic_error("get host info failed");
            if (pai != nullptr)
                peer = *((sockaddr_in*)pai->ai_addr);
        } else {
            vector<sockaddr_in> peers;
            discover(timeout, port, peers);
            if (peers.size() > 1) {
                int index;
                printf("You choice: ");
                scanf("%d", &index);
                if (index > 0 && index <= peers.size())
                    peer = peers[index-1];
            } else if (peers.size() == 1) {
                peer = peers[0];
            }
        }
        if (peer.sin_addr.s_addr != 0) {
            for (int i = 1; i < argc; ++i) {
                if (argv[i][0] == '-') {
                    i ++;
                    continue;
                }
                transfer(peer, argv[i]);
            }
        }
    } catch (std::exception const& ex) {
        fprintf(stderr, "%s\n", ex.what());
        status = EXIT_FAILURE;
    }
    return status;
}

void discover(uint32_t timeout, uint16_t port, vector<sockaddr_in> &peers) {
    socket_t sock(socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP), &closesocket);
    if (sock == INVALID_SOCKET)
        throw logic_error("socket failed");
    sockaddr_in addr{AF_INET}, server{AF_INET};
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        throw logic_error("bind failed");
    if (set_option(sock, SO_BROADCAST, true) == SOCKET_ERROR)
        throw logic_error("set broadcast failed");
    if (set_timeout(sock, SO_RCVTIMEO, timeout) == SOCKET_ERROR)
        throw logic_error("set timeout failed");
    char buf[BUFFER_SIZE];
    strcpy(buf, BROADCAST);
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    if (sendto(sock, buf, 1, 0, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        throw logic_error("send failed");
    socklen_t length = sizeof(server);
    ssize_t n;
    do {
        n = recvfrom(sock, buf, BUFFER_SIZE, 0, (sockaddr*)&server, &length);
        if (n == SOCKET_ERROR && errno == EAGAIN)
            break;
        if (server.sin_addr.s_addr == htonl(INADDR_LOOPBACK) && strcmp(buf, BROADCAST) == 0)
            continue;
        buf[n] = '\0';
        const auto& s = server.sin_addr;
        printf("%s %s\n", buf, inet_ntoa(s));
        peers.push_back(server);
    } while (true);
}

void transfer(const sockaddr_in &peer, const char *filename) {
    socket_t sock(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), &closesocket);
    if (sock == INVALID_SOCKET)
        throw logic_error("socket error");
    if (connect(sock, (sockaddr*)&peer, sizeof(peer)) == SOCKET_ERROR)
        throw logic_error("connect failed");
    try {
        if (set_timeout(sock, SO_SNDTIMEO, 3000) == SOCKET_ERROR)
            throw logic_error("set tcp timeout failed");
        file_t f(fopen(filename, "rb+"), &fclose);
        if (! f)
            throw logic_error("file open failed");
        fseek(f.get(), 0, SEEK_END);
        long size = ftell(f.get());
        fseek(f.get(), 0, SEEK_SET);
        char buf[BUFFER_SIZE];
        size_t sum = 0, progress = 0;
        size_t len;
        do {
            len = fread(buf,sizeof(char),BUFFER_SIZE,f.get());
            ::send(sock,buf,len,0);
            if ((sum += len) * 80 / size > progress) {
                while (sum * 80 / size > progress++)
                    putc('=', stdout);
                progress --;
            }
        } while(len >= BUFFER_SIZE);
        shutdown(sock, SHUT_WR);
    } catch (...) {
        shutdown(sock, SHUT_WR);
        throw;
    }
    char buf[BUFFER_SIZE];
    ssize_t len;
    while ((len = recv(sock, buf, BUFFER_SIZE, 0)) > 0) {
        for (int i = 0; i < len; ++i) {
            putc(buf[i], stdout);
        }
    }
}
