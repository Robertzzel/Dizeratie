#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
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
#include "scanner.h"

static uint8_t original_mac_ap[6];
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){}

int wifi_config(wifi_config_t* config) {
    return esp_wifi_set_config(ESP_IF_WIFI_AP, config);
}
int wifi_start() 
{
    int ret = esp_netif_init();
    if(ret != ESP_OK)
    {
        return ret;
    }

    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    ret = esp_wifi_init(&wifi_init_config);
    if(ret != ESP_OK)
    {
        return ret;
    }
    ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if(ret != ESP_OK)
    {
        return ret;
    }
    ret = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if(ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    if(ret != ESP_OK)
    {
        return ret;
    }
    ret = esp_wifi_get_mac(WIFI_IF_AP, original_mac_ap);
    if(ret != ESP_OK)
    {
        return ret;
    }
    ret = esp_wifi_start();
    if(ret != ESP_OK)
    {
        return ret;
    }
    return ESP_OK;
}
int wifi_stop(){
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = 0
        },
    };
    return esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
}

int disconnect(const char* str_bssid, wifictl_ap_records_t* ap_records) {
    uint8_t bssid[6];
    bssid_string_to_bytes(str_bssid, bssid);
    wifi_ap_record_t* record = get_ap_record_by_bssid(ap_records, bssid);
    if(record == NULL) {
        return -1;
    }
    int ret = esp_wifi_set_mac(WIFI_IF_AP, bssid);
    if(ret != ESP_OK) {
        return ret;
    }
    wifi_config_t config = {
        .ap = {
            .ssid_len = strlen((char*)record->ssid),
            .channel = record->primary,
            .authmode = record->authmode,
            .password = "ESP32ESP32",
            .max_connection = 1
        },
    };
    memcpy(config.ap.ssid, record->ssid, strlen((char*)record->ssid));
    printf("Setting ssid: %s\n", record->ssid);
    return wifi_config(&config);
}

#endif // WIFI_CONTROLLER_H