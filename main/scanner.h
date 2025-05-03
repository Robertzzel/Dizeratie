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

#define CONFIG_SCAN_MAX_AP 10
typedef struct {
    uint16_t count;
    wifi_ap_record_t records[CONFIG_SCAN_MAX_AP];
} wifictl_ap_records_t;

void print_ap_info(wifi_ap_record_t* ap) {
    printf("SSID: %s\n", ap->ssid);
    printf("Primary Channel: %u\n", ap->primary);
    printf("BSSID (MAC Address): %02X:%02X:%02X:%02X:%02X:%02X\n",
           ap->bssid[0], ap->bssid[1], ap->bssid[2],
           ap->bssid[3], ap->bssid[4], ap->bssid[5]);
}
void print_ap_records(wifictl_ap_records_t* ap_records){
    unsigned i;
    for(i = 0; i < ap_records->count; ++i){
        wifi_ap_record_t record = ap_records->records[i];
        print_ap_info(&record);
    }
}
int scan_networks(wifictl_ap_records_t* ap_records)
{
    ap_records->count = CONFIG_SCAN_MAX_AP;
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
    // BSSID must be 17 characters long: 6 pairs of hex digits, and 5 colons
    if (strlen(bssid) != 17) {
        return -1;  // Invalid BSSID length
    }

    for (int i = 0; i < 6; i++) {
        // Convert each pair of hex characters to a byte
        int high = hex_char_to_int(bssid[3*i]);         // First hex character of the pair
        int low = hex_char_to_int(bssid[3*i + 1]);      // Second hex character of the pair

        if (high == -1 || low == -1) {
            return -1;  // Invalid hex character in BSSID
        }

        // Combine the two nibbles into a byte and store it in the array
        bytes[i] = (high << 4) | low;
    }

    return 0;  // Success
}

char* records_to_json(wifictl_ap_records_t* ap){
    char* json = malloc(2048);
    if(json == NULL){
        return NULL;
    }
    int offset = 0;
    offset += snprintf(json + offset, 2048 - offset, "[");
    for(int i = 0; i < ap->count; i++){
        wifi_ap_record_t record = ap->records[i];
        offset += snprintf(json + offset, 2048 - offset, "{ \"ssid\": \"%s\", \"bssid\": \"%02X:%02X:%02X:%02X:%02X:%02X\" }",
                           record.ssid,
                           record.bssid[0], record.bssid[1], record.bssid[2],
                           record.bssid[3], record.bssid[4], record.bssid[5]);
        if(i < ap->count - 1){
            offset += snprintf(json + offset, 2048 - offset, ",");
        }
    }
    offset += snprintf(json + offset, 2048 - offset, "]");
    json[offset] = '\0';
    return json;
}
/*
ex scan:

wifictl_ap_records_t ap_records = {0};
int err = scan_networks(&state, &ap_records);
if (err != 0) {
    ESP_LOGE("app_main", "Failed to scan networks");
    return;
}

print_ap_records(&ap_records);
ESP_ERROR_CHECK(wifi_stop(&state));
*/
#endif // SCANNER_H