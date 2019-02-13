//
// Created by Lev on 13.02.2019.
//

#ifndef TFTP_SOCKET_H
#define TFTP_SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#define error WSAGetLastError()
#define ioctl ioctlsocket
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#typedef int SOCKET
#endif

#endif //TFTP_SOCKET_H
