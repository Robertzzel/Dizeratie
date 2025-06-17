#ifndef DNS_H
#define DNS_H

#define DNS_PORT 53
#define ESP32_IP "192.168.10.1"
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include "esp_log.h"

int get_query_name(const uint8_t *buffer, int len, char *domain, int max_len_domain);
void dns_server_task(void *pvParameters);

#endif // DNS_H