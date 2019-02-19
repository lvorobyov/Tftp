#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <thread>
#include <vector>
using namespace std;

#include <cxxopts.hpp>
using namespace cxxopts;

#define TFTP_PORT ((WORD)8969)

#define BUFFER_SIZE 512

#define BROADCAST "Broadcast"

void discover(DWORD timeout, WORD port, vector<sockaddr_in> &peers);

class transfer {
private:
    sockaddr_in peer;
    int step = 0;
    SOCKET sock = INVALID_SOCKET;
    FILE *f = nullptr;

public:
    explicit transfer(const sockaddr_in &peer) : peer(peer) {}

    virtual ~transfer() noexcept;

    void send(const char* filename);

    void cleanup() noexcept;
};

int main(int argc, char* argv[]) {
    WSADATA wsd;
    WSAStartup(MAKEWORD(2,2), &wsd);
    Options options(argv[0], "Transfer file client 1.0");
    options.add_options()
            ("r,host", "receiver host", value<string>())
            ("p,port", "receiver port", value<WORD>())
            ("t,timeout", "discover delay timeout", value<DWORD>())
            ("h,help", "show this help");
    auto result = options.parse(argc,argv);
    if (result.count("help")) {
        cout << options.help() << endl;
        return 0;
    }
    DWORD timeout = result.count("t")? result["t"].as<DWORD>() : 500;
    sockaddr_in peer{0};
    int status = EXIT_SUCCESS;
    try {
        auto port = result.count("port")? result["port"].as<WORD>() : TFTP_PORT;
        if (result.count("host")) {
            addrinfo hints{0};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            addrinfo *pai;
            char str_port[6];
            itoa(port, str_port, 10);
            if (getaddrinfo(result["host"].as<string>().c_str(), str_port, &hints, &pai) == SOCKET_ERROR)
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
            transfer tftp(peer);
            for (int i = 1; i < argc; ++i) {
                if (argv[i][0] == '-') {
                    i ++;
                    continue;
                }
                tftp.send(argv[1]);
            }
        }
    } catch (logic_error const& ex) {
        fprintf(stderr, "%s\n", ex.what());
        status = EXIT_FAILURE;
    }
    WSACleanup();
    return status;
}

void discover(DWORD timeout, WORD port, vector<sockaddr_in> &peers) {
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
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_BROADCAST);
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
        peers.push_back(server);
    } while (true);
    closesocket(sock);
}

void transfer::send(const char *filename) {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
        throw logic_error("socket error");
    step = 1;
    if (connect(sock, (sockaddr*)&peer, sizeof(peer)) == SOCKET_ERROR)
        throw logic_error("connect failed");
    step = 2;
    f = fopen(filename, "rb+");
    if (f == nullptr)
        throw logic_error("file not exists");
    step = 3;
    fseek(f, 0, SEEK_END);
    const int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char buf[BUFFER_SIZE];
    int sum = 0, progress = 0;
    int len;
    do {
        len = fread(buf,sizeof(char),BUFFER_SIZE,f);
        ::send(sock,buf,len,0);
        if ((sum += len) * 80 / size > progress) {
            while (sum * 80 / size > progress++)
                printf("=");
            progress --;
        }
    } while(len >= BUFFER_SIZE);
    cleanup();
}

transfer::~transfer() noexcept {
    cleanup();
}

void transfer::cleanup() noexcept {
    switch (step) {
        case 3:
            fclose(f);
            f = nullptr;
        case 2:
            shutdown(sock, SD_SEND);
        case 1:
            closesocket(sock);
            sock = INVALID_SOCKET;
        default:break;
    }
    step = 0;
}
