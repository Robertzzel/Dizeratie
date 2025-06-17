#ifndef SOCKET_H
#define SOCKET_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <errno.h>
#include <stddef.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

enum AddressFamily {
    IPv4 = AF_INET,
    IPv6 = AF_INET6
};

struct Address {
    char* host;
    int port;
    enum AddressFamily family;
};

enum SocketType {
    Stream = SOCK_STREAM,
    Datagram = SOCK_DGRAM,
};

int socket_create(enum AddressFamily family, enum SocketType type);
int socket_set_reuse_addr_option(int sock);
int socket_bind(int sock, enum AddressFamily family, int port, int host);
int socket_connect(int sock, enum AddressFamily family, int port, const char* host);
int socket_listen(int sock, int numberOfClients);
int socket_accept(int sock);
int socket_send(int sock, char* buffer, int bytesToWrite);
size_t socket_receive(int sock, void* buffer, size_t bytes_to_receive);
int socket_close(int sock);
int socket_get_last_error();
#endif