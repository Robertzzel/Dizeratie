#ifndef SCANNER_H
#define SCANNER_H
#include "esp_log.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <nvs_flash.h>
#include "allocator.h"

typedef struct wifi_ap_records_t {
    wifi_ap_record_t* records;
    uint16_t count;
} AP_records_t;

AP_records_t* scan_networks(allocator_t* allocator, uint16_t APs)
{
    AP_records_t* ap_records = allocator_alloc_type(allocator, AP_records_t, 1);
    if (ap_records == NULL) {
        ESP_LOGE("Scanner", "Failed to allocate memory for AP records");
        return NULL;
    }
    ap_records->records = allocator_alloc_type(allocator, wifi_ap_record_t, APs);
    if (ap_records->records == NULL) {
        ESP_LOGE("Scanner", "Failed to allocate memory for AP records array");
        return NULL;
    }
    ap_records->count = APs;

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE
    };
    scan_config.scan_time.active.min = 70; // Minimum scan time in ms
    scan_config.scan_time.active.max = 74; // Maximum scan time in ms
    int ret = esp_wifi_scan_start(&scan_config, true);
    if(ret != ESP_OK) {
        ESP_LOGE("Scanner", "Failed to start WiFi scan: %s", esp_err_to_name(ret));
        return NULL;
    }
    ret =  esp_wifi_scan_get_ap_records(&ap_records->count, ap_records->records);
    if(ret != ESP_OK) {
        ESP_LOGE("Scanner", "Failed to get AP records: %s", esp_err_to_name(ret));
        return NULL;
    }
    allocator_realloc_type(allocator, ap_records->records, wifi_ap_record_t, APs);
    return ap_records;
}

wifi_ap_record_t* get_ap_record_by_bssid(AP_records_t* ap_records, const unsigned char* bssid) {
    for (int i = 0; i < ap_records->count; i++) {
        for (int j = 0; j < 6; j++) {
            
            if (ap_records->records[i].bssid[j] != bssid[j]) {
                break;
            }
            if (j == 5) {
                return &ap_records->records[i];
            }
        }
    }
    return NULL;
}


int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;  // Invalid hex character
}

uint8_t* bssid_string_to_bytes(allocator_t* allocator, const char *bssid) {
    if (strlen(bssid) != 17) {
        return NULL;
    }
    uint8_t* bytes = allocator_alloc_type(allocator, uint8_t, 6);
    if (!bytes) {
        return NULL;
    }

    for (int i = 0; i < 6; i++) {
        int high = hex_char_to_int(bssid[3*i]);
        int low = hex_char_to_int(bssid[3*i + 1]);

        if (high == -1 || low == -1) {
            return NULL;
        }

        bytes[i] = (high << 4) | low;
    }

    return bytes;  // Success
}

char* records_to_json(allocator_t* arena, AP_records_t* ap) {
    if (!ap || !arena || ap->count == 0) return NULL;
    
    size_t total_size = 63 * ap->count + 2 /*[]*/ + ap->count-1 /*,*/;
    for (int i = 0; i < ap->count; ++i) {
        total_size += strlen((char*)ap->records[i].ssid);
        total_size += 4; // channel + authmode
    }
    char* buf = allocator_alloc_type(arena, char, total_size+1);
    if (!buf) return NULL;
    buf[0] = 0;
    size_t offset = 0;

    buf[offset++] = '[';

    char temp[256];
    for (int i = 0; i < ap->count; ++i) {
        wifi_ap_record_t r = ap->records[i];

        // Format one record to temp buffer
        int len = snprintf(temp, sizeof(temp),
            "{\"ssid\":\"%s\",\"bssid\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"channel\":%d,\"authmode\":%d}", // 63 + ssid
            r.ssid,
            r.bssid[0], r.bssid[1], r.bssid[2],
            r.bssid[3], r.bssid[4], r.bssid[5],
            r.primary, r.authmode);

        if (len < 0) return NULL;

        memcpy(buf + offset, temp, len);
        offset += len;

        if (i < ap->count - 1) {
            buf[offset++] = ',';
        }
    }

    buf[offset++] = ']';
    buf[offset] = '\0';

    return buf;
}

#endif // SCANNER_H