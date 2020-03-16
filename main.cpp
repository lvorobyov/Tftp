#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cerrno>
typedef int SOCKET;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef bool BOOL;
#define TRUE true
typedef u_int16_t WORD;
typedef u_int32_t DWORD;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SD_SEND SHUT_WR
#define closesocket ::close
#define ioctlsocket ::ioctl
inline int WSAGetLastError() { return errno; }
#define WSAETIMEDOUT EAGAIN
#endif
#include <cstdio>
#include <thread>
#include <vector>
using namespace std;

#include <cxxopts.hpp>
using namespace cxxopts;

#define TFTP_PORT ((WORD)8969)

#define BUFFER_SIZE 512

#define BROADCAST "Broadcast"

void discover(timeval timeout, WORD port, vector<sockaddr_in> &peers);

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

    void close() noexcept;
};

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsd;
    WSAStartup(MAKEWORD(2,2), &wsd);
#endif
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
            sprintf(str_port, "%d", port);
            if (getaddrinfo(result["host"].as<string>().c_str(), str_port, &hints, &pai) == SOCKET_ERROR)
                throw logic_error("get host info failed");
            if (pai != nullptr)
                peer = *((sockaddr_in*)pai->ai_addr);
        } else {
            vector<sockaddr_in> peers;
            discover({timeout / 1000, (timeout % 1000) * 1000}, port, peers);
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
                tftp.send(argv[i]);
            }
        }
    } catch (exception const& ex) {
        fprintf(stderr, "%s\n", ex.what());
        status = EXIT_FAILURE;
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return status;
}

void discover(timeval timeout, WORD port, vector<sockaddr_in> &peers) {
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
    socklen_t length = sizeof(server);
    int n;
    do {
        n = recvfrom(sock, buf, BUFFER_SIZE, 0, (sockaddr*)&server, &length);
        if (n == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
            break;
        if (server.sin_addr.s_addr == htonl(INADDR_LOOPBACK) && strcmp(buf, BROADCAST) == 0)
            continue;
        buf[n] = '\0';
        const auto& s = server.sin_addr;
        printf("%s %s\n", buf, inet_ntoa(s));
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
    timeval timeout {3, 0};
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
        throw logic_error("set tcp timeout failed");
    step = 2;
    f = fopen(filename, "rb+");
    if (f == nullptr)
        throw system_error(errno,system_category(),"file open failed");
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
                putc('=', stdout);
            progress --;
        }
    } while(len >= BUFFER_SIZE);
    cleanup();
    while ((len = ::recv(sock,buf,BUFFER_SIZE, 0)) != 0) {
        for (int i = 0; i < len; ++i) {
            putc(buf[i], stdout);
        }
    }
    close();
}

transfer::~transfer() noexcept {
    cleanup();
    close();
}

void transfer::cleanup() noexcept {
    switch (step) {
        case 3:
            fclose(f);
            f = nullptr;
        case 2:
            shutdown(sock, SD_SEND);
        default:break;
    }
    step = 0;
}

void transfer::close() noexcept {
    closesocket(sock);
    sock = INVALID_SOCKET;
}
