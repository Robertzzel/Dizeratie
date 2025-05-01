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
#include "state.h"
#include "webserver.h"
static state_t state = {0};

void app_main(void)
    {
    printf("app_main started\n");

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    state.event_loop_started = true;
    ESP_ERROR_CHECK(nvs_flash_init());
    state.nvs_flash_initialized = true;

    ESP_ERROR_CHECK(wifi_init_start(&state));

    start_webserver();
}