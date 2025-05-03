#include <stdio.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
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
#include "wifi_controller.h"
#include "scanner.h"
#include "webserver.h"

void app_main(void)
    {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(wifi_start());

    wifi_config_t config = 
    {
        .ap = 
        {
            .ssid = "ESP32",
            .ssid_len = strlen("ESP32"),
            .password = "ESP32ESP32",
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(wifi_config(&config));

    start_webserver();
}