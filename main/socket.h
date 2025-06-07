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

int socket_create(enum AddressFamily family, enum SocketType type) {
    return socket(family, type, IPPROTO_IP);
}
int socket_set_reuse_addr_option(int sock) {
    int opt = 1;
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}
int socket_bind(int sock, enum AddressFamily family, int port, int host) {
    struct sockaddr_in dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = host; //inet_addr
    dest_addr_ip4->sin_family = family;
    dest_addr_ip4->sin_port = htons(port);

    return bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
}
int socket_connect(int sock, enum AddressFamily family, int port, const char* host){
    struct sockaddr_in dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = inet_addr(host);
    dest_addr_ip4->sin_family = family;
    dest_addr_ip4->sin_port = htons(port);
    return connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
}
int socket_listen(int sock, int numberOfClients) {
    return listen(sock, numberOfClients);
}
int socket_accept(int sock) {
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);
    return accept(sock, (struct sockaddr *)&source_addr, &addr_len);
}
int socket_send(int sock, char* buffer, int bytesToWrite) {
    return send(sock, buffer, bytesToWrite, 0);
}
size_t socket_receive(int sock, void* buffer, size_t bytes_to_receive) {
    return recv(sock, buffer, bytes_to_receive, 0);
}
int socket_close(int sock) {
    return close(sock);
}
int socket_get_last_error(){
    return errno;
}

#endif // SOCKET_H