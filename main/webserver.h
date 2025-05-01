#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "socket.h"

static wifictl_ap_records_t ap_records = {0};

const char html[] = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Document</title></head><body><table id=\"data-table\" border=\"1\"><thead><tr><th>Column 1</th><th>Column 2</th><th>Column 3</th></tr></thead><tbody id=\"table-body\"></tbody></table><button id=\"fetch-data\">Fetch Data</button></body><script>document.getElementById('fetch-data').addEventListener('click',async()=>{try{console.log(\"fetching data...\");const response=await fetch('/scan');if(!response.ok){throw new Error('Network response was not ok');}console.log(\"parsing response to json...\");const data=await response.json();console.log(\"fetching data...\");const tableBody=document.getElementById('table-body');tableBody.innerHTML='';data.forEach(row=>{const tr=document.createElement('tr');let td=document.createElement('td');td.textContent=row.ssid;tr.appendChild(td);td=document.createElement('td');td.textContent=row.bssid;tr.appendChild(td);tableBody.appendChild(tr);});}catch(error){console.error('Error fetching data:',error);}});</script></html>";

typedef struct {
    char method[8];
    char url[256];
} http_request_t;

http_request_t parse_http_request(const char* request) {
    http_request_t req;
    sscanf(request, "%s %s", req.method, req.url);
    return req;
}

void handle_connection(http_request_t* req, int client_sock) {
    if (strcmp(req->method, "GET") == 0 && strcmp(req->url, "/") == 0) {
        // send index.html file
        socket_send(client_sock, html, strlen(html));
    } else if (strcmp(req->method, "GET") == 0 && strcmp(req->url, "/scan") == 0) {
        // handle scan request
        ESP_LOGE("WebServer", "Handling scan request");
        
        int err = scan_networks(&ap_records);
        if (err != 0) {
            ESP_LOGE("WebServer", "Failed to scan networks");
            return;
        }
        ESP_LOGE("WebServer", "Scan completed");
        char* records = records_to_json(&ap_records);
        printf("Records:%s\n", records);
        socket_send(client_sock, records, strlen(records));
        free(records);
    } else {
        // send 404 response
        
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
    if (socket_bind(sock, IPv4, 80, INADDR_ANY) < 0) {
        ESP_LOGE("WebServer", "Failed to bind socket");
        close(sock);
        return;
    }

    ESP_LOGI("WebServer", "Listening...");
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
        buffer[len] = '\0';
        
        http_request_t req = parse_http_request(buffer);
        printf("Request Method: %s\n", req.method);
        printf("Request URL: %s\n", req.url);

        handle_connection(&req, client_sock);

        close(client_sock);
    }
}

#endif // WEBSERVER_H