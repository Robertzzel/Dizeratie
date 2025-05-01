#include "wifi_controller.h"

static bool is_wifi_inited = false;
static uint8_t original_mac_ap[6];

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){}
int wifi_init_start(state_t* state)
{
    if(state->event_loop_started == false || state->nvs_flash_initialized == false)
    {
        return ESP_FAIL;
    }

    wifi_config_t mgmt_wifi_config = 
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
    int ret = wifi_start(state);
    if(ret != ESP_OK)
    {
        return ret;
    }

    return esp_wifi_set_config(ESP_IF_WIFI_AP, &mgmt_wifi_config);
}

int wifi_start(state_t* state) 
{
    if(state->event_loop_started == false || state->nvs_flash_initialized == false || state->wifi_started == true)
    {
        return ESP_FAIL;
    }

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
    state->wifi_started = true;
    return ESP_OK;
}

int wifi_stop(state_t* state){
    if(state->wifi_started == false)
    {
        return ESP_FAIL;
    }
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = 0
        },
    };
    return esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
}