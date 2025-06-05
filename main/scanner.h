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

#define MAX_APS 15
typedef struct {
    uint16_t count;
    wifi_ap_record_t records[MAX_APS];
} wifictl_ap_records_t;

int scan_networks(wifictl_ap_records_t* ap_records)
{
    ap_records->count = MAX_APS;
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE
    };
    
    int ret = esp_wifi_scan_start(&scan_config, true);
    if(ret != ESP_OK) {
        return -1;
    }
    ret =  esp_wifi_scan_get_ap_records(&ap_records->count, ap_records->records);
    if(ret != ESP_OK) {
        return -1;
    }
    return 0;
}

wifi_ap_record_t* get_ap_record_by_bssid(wifictl_ap_records_t* ap_records, const unsigned char* bssid) {
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

int bssid_string_to_bytes(const char *bssid, unsigned char *bytes) {
    if (strlen(bssid) != 17) {
        return -1;
    }

    for (int i = 0; i < 6; i++) {
        int high = hex_char_to_int(bssid[3*i]);
        int low = hex_char_to_int(bssid[3*i + 1]);

        if (high == -1 || low == -1) {
            return -1;
        }

        bytes[i] = (high << 4) | low;
    }

    return 0;  // Success
}

int records_to_json(wifictl_ap_records_t* ap, char* buf, size_t bufsize) {
    if (buf == NULL || bufsize == 0) {
        return 1;
    }
    int offset = 0;
    int written = 0;
    written = snprintf(buf + offset, bufsize - offset, "[");
    if (written < 0 || written >= bufsize - offset)
        return 2;
    offset += written;

    for (int i = 0; i < ap->count; i++) {
        wifi_ap_record_t record = ap->records[i];
        written = snprintf(buf + offset, bufsize - offset,
            "{ \"ssid\": \"%s\", \"bssid\": \"%02X:%02X:%02X:%02X:%02X:%02X\" }",
            record.ssid,
            record.bssid[0], record.bssid[1], record.bssid[2],
            record.bssid[3], record.bssid[4], record.bssid[5]);
        if (written < 0 || written >= bufsize - offset)
            return 2;
        offset += written;

        if (i < ap->count - 1) {
            written = snprintf(buf + offset, bufsize - offset, ",");
            if (written < 0 || written >= bufsize - offset)
                return 2;
            offset += written;
        }
    }
    written = snprintf(buf + offset, bufsize - offset, "]");
    if (written < 0 || written >= bufsize - offset)
        return 2;
    offset += written;

    return 0;
}

#endif // SCANNER_H