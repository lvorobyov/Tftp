//
// Created by Lev on 12.02.2019.
//

#include <cstdio>
#include <cstdint>
#include <csignal>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#define error WSAGetLastError()
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#typedef int SOCKET
#endif
#include <stdexcept>
using namespace std;

#define BUFFER_SIZE 512
#define TFTP_PORT 8969

class tftp {
private:
    int state = 0;
    SOCKET sock = INVALID_SOCKET;

public:
    void start();

	~tftp() noexcept {
		cleanup();
	}

protected:
    void cleanup() noexcept;
};

int main(int argc, char* argv[]) {
    tftp server;
	try {
		server.start();
	} catch(const logic_error& ex) {
		printf("%s: %d\n", ex.what(), error);
		return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}

void tftp::start() {
    char buf[BUFFER_SIZE];
#ifdef _WIN32
    WSADATA wsd{0};
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
        throw logic_error("startup fail");
    state = 1;
#endif
    sockaddr_in addr{AF_INET};
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(TFTP_PORT);
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
        throw logic_error("socket error");
    state = 2;
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        throw logic_error("bind failed");
    BOOL is_broadcast = TRUE;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&is_broadcast, sizeof(is_broadcast)) == SOCKET_ERROR)
        throw logic_error("option setting error");
    static sig_atomic_t active = true;
    signal(SIGINT, [](int s) { active = false; });

    sockaddr_in client{};
    int length = sizeof(client);
    do {
        int n = recvfrom(sock, buf, BUFFER_SIZE, 0,
                         (sockaddr*)&client, &length);
        if (n == SOCKET_ERROR)
            throw logic_error("receive failed");
        printf("%s\n", buf);
    } while(active);
}

void tftp::cleanup() noexcept {
    printf("Bue!\n");
    switch(state) {
        case 2:
            close(sock);
#ifdef _WIN32
        case 1:
            WSACleanup();
        default:
            break;
#endif
    }
}
