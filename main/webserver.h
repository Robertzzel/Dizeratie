#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "socket.h"
#include "http_response.h"
#include "http_request.h"
#include "html_pages.h"
#include "tasks.h"
#include "facebook_web_server.h"

#define MAIN_WEBSERVER_BUFFER_SIZE 1024 * 8
uint8_t main_webserver_buffer[MAIN_WEBSERVER_BUFFER_SIZE];
char json[MAX_CRED_ENDTRIES_JSON] = {0};

void handle_scan(allocator_t* allocator, int client_sock);

void handle_attack(allocator_t* allocator, http_request_t* req, int client_sock);

void handle_flood(http_request_t* req, int client_sock);
void handle_flood_stop(http_request_t* req, int client_sock);
void handle_facebook_data(http_request_t* req, int client_sock);

void handle_connection(allocator_t* allocator, http_request_t* req, int client_sock);

void start_webserver();


#endif // WEBSERVER_H