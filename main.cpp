#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <thread>
#include <vector>
using namespace std;

#include <cxxopts.hpp>
using namespace cxxopts;

#define TFTP_PORT 8969

#define BUFFER_SIZE 512

#define BROADCAST "Broadcast"

void discover(DWORD timeout, vector<in_addr> &peers);

void transfer(in_addr peer, char *filename);

int main(int argc, char* argv[]) {
    WSADATA wsd;
    WSAStartup(MAKEWORD(2,2), &wsd);
    Options options(argv[0], "Transfer file client 1.0");
    options.add_options()
            ("r,host", "receiver host", value<string>())
            ("t,timeout", "discover delay timeout", value<DWORD>())
            ("h,help", "show this help");
    auto result = options.parse(argc,argv);
    if (result.count("help")) {
        cout << options.help() << endl;
        return 0;
    }
    DWORD timeout = result.count("t")? result["t"].as<DWORD>() : 500;
    in_addr peer{0};
    int status = EXIT_SUCCESS;
    try {
        if (result.count("host")) {
            addrinfo hints{0};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            addrinfo *pai;
            if (getaddrinfo(result["host"].as<string>().c_str(), "8969", &hints, &pai) == SOCKET_ERROR)
                throw logic_error("get host info failed");
            if (pai != nullptr)
                peer = ((sockaddr_in*)pai->ai_addr)->sin_addr;
        } else {
            vector<in_addr> peers;
            discover(timeout, peers);
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
        if (peer.s_addr != 0) {
            for (int i = 1; i < argc; ++i) {
                if (argv[i][0] == '-') {
                    i ++;
                    continue;
                }
                transfer(peer, argv[1]);
            }
        }
    } catch (logic_error const& ex) {
        fprintf(stderr, "%s\n", ex.what());
        status = EXIT_FAILURE;
    }
    WSACleanup();
    return status;
}

void discover(DWORD timeout, vector<in_addr> &peers) {
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
        throw logic_error("socket failed");
    sockaddr_in addr{AF_INET}, server{AF_INET};
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        throw logic_error("bind failed");
    BOOL is_broadcast = TRUE;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&is_broadcast, sizeof(is_broadcast)) == SOCKET_ERROR)
        throw logic_error("set broadcast failed");
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        throw logic_error("set timeout failed");
    char buf[BUFFER_SIZE];
    strcpy(buf, BROADCAST);
    server.sin_port = htons(TFTP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (sendto(sock, buf, 1, 0, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        throw logic_error("send failed");
    int length = sizeof(server);
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
	fseek(f, 0, SEEK_END);
	const int size = ftell(f);
	fseek(f, 0, SEEK_SET);
    char buf[BUFFER_SIZE];
	int count = 0, progress = 0;
    int len;
    do {
		while (count * 80 * BUFFER_SIZE / size > progress) {
			printf("=");
			progress++;
		}
		count ++;
        len = fread(buf,sizeof(char),BUFFER_SIZE,f);
        send(sock,buf,len,0);
    } while(len >= BUFFER_SIZE);
    fclose(f);
    if (shutdown(sock, SD_SEND) == SOCKET_ERROR)
        throw logic_error("shutdown failed");
    closesocket(sock);
}