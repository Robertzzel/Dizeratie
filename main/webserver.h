#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "socket.h"

static wifictl_ap_records_t ap_records = {0};

char* html = 
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>Document</title>\n"
"</head>\n"
"<body>\n"
"    <table id=\"data-table\" border=\"1\">\n"
"        <thead>\n"
"            <tr>\n"
"                <th>SSID</th>\n"
"                <th>BSSID</th>\n"
"                <th>DISCONNECT</th>\n"
"            </tr>\n"
"        </thead>\n"
"        <tbody id=\"table-body\">\n"
"            <!-- Rows will be dynamically added here -->\n"
"        </tbody>\n"
"    </table>\n"
"    <button id=\"fetch-data\">Fetch Data</button>\n"
"</body>\n"
"<script>\n"
"    document.getElementById('fetch-data').addEventListener('click', async () => {\n"
"        try {\n"
"            console.log(\"fetching data...\");\n"
"            const response = await fetch('/scan');\n"
"            if (!response.ok) {\n"
"                throw new Error('Network response was not ok');\n"
"            }\n"
"            console.log(\"parsing response to json...\");\n"
"            const data = await response.json();\n"
"            console.log(\"data: \", data)\n"
"            console.log(\"populating table...\");\n"
"            populate_table(data);\n"
"        } catch (error) {\n"
"            console.error('Error fetching data:', error);\n"
"        }\n"
"    });\n"
"\n"
"    function populate_table(json_data){\n"
"        const tableBody = document.getElementById('table-body');\n"
"        tableBody.innerHTML = ''; // Clear existing rows\n"
"\n"
"        json_data.forEach(row => {\n"
"            const tr = document.createElement('tr');\n"
"\n"
"            let td = document.createElement('td');\n"
"            td.textContent = row.ssid;\n"
"            tr.appendChild(td);\n"
"\n"
"            td = document.createElement('td');\n"
"            td.textContent = row.bssid;\n"
"            tr.appendChild(td);\n"
"\n"
"            td = document.createElement('td');\n"
"            td.innerHTML = `<button onclick=\"disconnect('${row.bssid}')\">disconnect</button>`;\n"
"            tr.appendChild(td);\n"
"\n"
"            tableBody.appendChild(tr);\n"
"        });\n"
"    }\n"
"\n"
"    async function disconnect(target_bssid){\n"
"        const response = await fetch(`/attack?bssid=${target_bssid}`);\n"
"    }\n"
"</script>\n"
"</html>\n";

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
    } else if(strcmp(req->method, "GET") == 0 && strncmp(req->url, "/attack", 7) == 0) {
        // get bssid from url
        char bssid[18];
        sscanf(req->url, "/attack?bssid=%s", bssid);
        bssid[18] = '\0';
        // start attack
        ESP_LOGI("WebServer", "Starting attack on BSSID: %s", bssid);
        int ret = disconnect(bssid, &ap_records);
        if(ret != ESP_OK) {
            ESP_LOGE("WebServer", "BSSID not found");
            char* not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            socket_send(client_sock, not_found, strlen(not_found));
            return;
        }
        char* found = "HTTP/1.1 200 Ok\r\nContent-Length: 0\r\n\r\n";
        socket_send(client_sock, found, strlen(found));
    }
    else {
        char* not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        socket_send(client_sock, not_found, strlen(not_found));
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