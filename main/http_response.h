#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H


int http_send_json_response(int client_sock, const char* json) {
    size_t response_size = strlen(json) + 128;
    char* response = (char*)malloc(response_size);
    if (!response) {
        return -1;
    }
    snprintf(response, response_size, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", strlen(json), json);
    socket_send(client_sock, response, strlen(response));
    free(response);
    return 0;
}

int http_send_html_response(int client_sock, const char* html) {
    size_t header_size = 128;
    char* response = (char*)malloc(header_size);
    if (!response) {
        return -1;
    }
    snprintf(response, header_size, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n", strlen(html));
    socket_send(client_sock, response, strlen(response));
    socket_send(client_sock, html, strlen(html));
    free(response);
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


#endif // HTTP_RESPONSE_H