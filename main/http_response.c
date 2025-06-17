#include "http_response.h"
#include "socket.h"
#include <stdint.h>

int http_send_json_response(int client_sock, char* json) {
    size_t response_size = 256;
    char response[256];
    snprintf(response, response_size, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n", strlen(json));
    socket_send(client_sock, response, strlen(response));
    socket_send(client_sock, json, strlen(json));
    return 0;
}

int http_send_html_response(int client_sock, char* html) {
    size_t header_size = 128;
    char response[128];
    snprintf(response, header_size, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n", strlen(html));
    socket_send(client_sock, response, strlen(response));
    socket_send(client_sock, html, strlen(html));
    return 0;
}

int http_send_not_found_response(int client_sock) {
    char* not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    socket_send(client_sock, not_found, strlen(not_found));
    return 0;
}
int http_send_ok_response(int client_sock) {
    char* ok = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    socket_send(client_sock, ok, strlen(ok));
    return 0;
}
int http_send_error_response(int client_sock) {
    char* error = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
    socket_send(client_sock, error, strlen(error));
    return 0;
}
int http_send_bad_request_response(int client_sock) {
    char* bad_request = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
    socket_send(client_sock, bad_request, strlen(bad_request));
    return 0;
}
int http_send_conflict_response(int client_sock) {
    char* conflict = "HTTP/1.1 409 Conflict\r\nContent-Length: 0\r\n\r\n";
    socket_send(client_sock, conflict, strlen(conflict));
    return 0;
}