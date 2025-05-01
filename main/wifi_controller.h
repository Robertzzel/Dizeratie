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
#include "state.h"

int wifi_init_start(state_t* state);
int wifi_start(state_t* state);
int wifi_stop(state_t* state);

#endif // WIFI_CONTROLLER_H