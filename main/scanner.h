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
#include "state.h"

#define CONFIG_SCAN_MAX_AP 10
typedef struct {
    uint16_t count;
    wifi_ap_record_t records[CONFIG_SCAN_MAX_AP];
} wifictl_ap_records_t;

void print_ap_info(wifi_ap_record_t* ap);
void print_ap_records(wifictl_ap_records_t* ap_records);
int scan_networks(wifictl_ap_records_t*);
char* records_to_json(wifictl_ap_records_t* ap);
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