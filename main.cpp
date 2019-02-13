#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <thread>
using namespace std;

#define TFTP_PORT 8969

#define BUFFER_SIZE 512

int main() {
    WSADATA wsd;
    WSAStartup(MAKEWORD(2,2), &wsd);
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
        return 1;
    sockaddr_in addr{AF_INET}, server{AF_INET};
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        return 1;
    BOOL is_broadcast = TRUE;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&is_broadcast, sizeof(is_broadcast)) == SOCKET_ERROR)
        return 1;
    DWORD timeout = 500;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        return 1;
    char buf[BUFFER_SIZE];
    server.sin_port = htons(TFTP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (sendto(sock, buf, 1, 0, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        return 1;
    int length = sizeof(server);
    int n;
    do {
        n = recvfrom(sock, buf, BUFFER_SIZE, 0, (sockaddr*)&server, &length);
        if (n == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
            break;
        buf[n] = '\0';
        const auto& s = server.sin_addr;
        printf("%s %d.%d.%d.%d\n", buf, s.s_net, s.s_host, s.s_lh, s.s_impno);
    } while (true);
    closesocket(sock);
    WSACleanup();
    return 0;
}