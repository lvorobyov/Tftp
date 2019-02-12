#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>

#define TFTP_PORT 8969

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
    const char* buf = "Broadcast";
    server.sin_port = htons(TFTP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (sendto(sock, buf, static_cast<int>(strlen(buf)), 0, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        return 1;
    return 0;
}