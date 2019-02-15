//
// Created by Lev on 12.02.2019.
//

#include <cstdio>
#include <cstdint>
#include <csignal>
#include <unistd.h>
#include "socket.h"
#include <stdexcept>
using namespace std;

#include "receiver.h"
using namespace tftp;

#define BUFFER_SIZE 512
#define TFTP_PORT 8969

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#define SERVICE_NAME "Transfer file service"
#endif

class tftps {
private:
    int state = 0;
    SOCKET sock = INVALID_SOCKET;

public:
    void start();

	~tftps() noexcept {
		cleanup();
	}

protected:
    void cleanup() noexcept;
};

void WINAPI service_main(DWORD,LPCTSTR);

int main(int argc, char* argv[]) {
    char service_name[] = SERVICE_NAME;
    SERVICE_TABLE_ENTRY service_table[] {
            {service_name, (LPSERVICE_MAIN_FUNCTION)service_main},
            {nullptr, nullptr}
    };
    if (! StartServiceCtrlDispatcher(service_table)) {
        printf("%s: %d\n", "StartServiceCtrlDispatcher", static_cast<int>(GetLastError()));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void WINAPI service_control(DWORD dwCode);

void WINAPI service_main(DWORD, LPCTSTR) {
    SERVICE_STATUS_HANDLE  status_handle =
            RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION) service_control);
    tftps server;
    try {
        server.start();
    } catch(const logic_error& ex) {
        printf("%s: %d\n", ex.what(), error);
    }
}

void WINAPI service_control(DWORD dwCode) {
    switch (dwCode) {
        case SERVICE_CONTROL_STOP:
            // Stop service
            break;
        case SERVICE_CONTROL_INTERROGATE:
            //SetServiceStatus(status_handle);
            break;
        default:
            break;
    }
}

void tftps::start() {
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
    static volatile sig_atomic_t active = true;
    signal(SIGINT, [](int s) { active = false; });
	
	DWORD timeout = 500;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        throw logic_error("timeout setting error");

	receiver thread1;
	thread1.start();
    sockaddr_in client{};
    int length = sizeof(client);
    do {
        int n = recvfrom(sock, buf, BUFFER_SIZE, 0,
                         (sockaddr*)&client, &length);
        if (n == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAETIMEDOUT)
				throw logic_error("receive failed");
			else continue;
		}
        char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);
        if (sendto(sock, hostname, strlen(hostname), 0, (sockaddr*)&client, length) == SOCKET_ERROR)
            throw logic_error("send failed");
    } while(active);
    thread1.stop();
    thread1.wait();
}

void tftps::cleanup() noexcept {
    printf("Bye!\n");
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
