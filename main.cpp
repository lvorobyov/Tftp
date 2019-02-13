#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <thread>
#include <vector>
using namespace std;

#define TFTP_PORT 8969

#define BUFFER_SIZE 512

#define BROADCAST "Broadcast"

void transfer(in_addr peer, char *filename);

int main(int argc, char* argv[]) {
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
    strcpy(buf, BROADCAST);
    server.sin_port = htons(TFTP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (sendto(sock, buf, 1, 0, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        return 1;
    int length = sizeof(server);
    vector<in_addr> peers;
    int n;
    do {
        n = recvfrom(sock, buf, BUFFER_SIZE, 0, (sockaddr*)&server, &length);
        if (n == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
            break;
        if (server.sin_addr.s_addr == htonl(INADDR_LOOPBACK) && strcmp(buf, BROADCAST) == 0)
            continue;
        buf[n] = '\0';
        const auto& s = server.sin_addr;
        printf("%s %d.%d.%d.%d\n", buf, s.s_net, s.s_host, s.s_lh, s.s_impno);
        peers.push_back(s);
    } while (true);
    closesocket(sock);
    if (argc > 1) {
        int index;
        printf("You choice: ");
        scanf("%d", &index);
        if (index > 0 && index < peers.size()) {
            for (int i = 1; i < argc; ++i) {
                try {
                    transfer(peers[index], argv[1]);
                } catch (logic_error const& ex) {
                    fprintf(stderr, "%s\n", ex.what());
                    break;
                }
            }
        }
    }
    WSACleanup();
    return 0;
}

void transfer(in_addr peer, char *filename) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
        throw logic_error("socket error");
    sockaddr_in addr {PF_INET};
    addr.sin_port = htons(TFTP_PORT);
    addr.sin_addr = peer;
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        throw logic_error("connect failed");
    }
    FILE *f = fopen(filename, "rb+");
    char buf[BUFFER_SIZE];
    int len;
    do {
        len = fread(buf,sizeof(char),BUFFER_SIZE,f);
        send(sock,buf,len,0);
    } while(len >= BUFFER_SIZE);
    fclose(f);
    if (shutdown(sock, SD_SEND) == SOCKET_ERROR)
        throw logic_error("shutdown failed");
}