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

AP_records_t* scan_networks(allocator_t* allocator, uint16_t APs);
wifi_ap_record_t* get_ap_record_by_bssid(AP_records_t* ap_records, const unsigned char* bssid);
int hex_char_to_int(char c);
uint8_t* bssid_string_to_bytes(allocator_t* allocator, const char *bssid);
char* records_to_json(allocator_t* arena, AP_records_t* ap);

#endif // SCANNER_H