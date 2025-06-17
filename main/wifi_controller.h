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
#include "esp_timer.h"
#include <nvs_flash.h>
#include "lwip/inet.h"
#include "scanner.h"
#include "os_functions.h"
#include "lwip/ip4_addr.h"
#include <arpa/inet.h>

static uint8_t original_mac_ap[6];
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){}

uint8_t beacon_frame[] = {
	0x80, 0x00,							                        // 0-1: Frame Control
	0x00, 0x00,							                        // 2-3: Durata
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff,				            // 4-9: Destiantia (broadcast)
	0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06,				            // 10-15: Sursa
	0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06,				            // 16-21: BSSID
    0x00, 0x00,							                        // 22-23: Secventa(nr fragmentului)
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	            // 24-31: Timestamp
	0x64, 0x00,							                        // 32-33: Beacon interval
	0x31, 0x04,							                        // 34-35: Capability info
	0x00, 0x00, /* <BEACON_SSID_OFFSET> */                      // 36-38: SSID, 0x00-length-data
	0x01, 0x08, 0x82, 0x84,	0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,	// 39-48: Rates
	0x03, 0x01, 0x01,						                    // 49-51: DS Parameter set
	0x05, 0x04, 0x01, 0x02, 0x00, 0x00,				            // 52-57: Traffic Indication Map
};
#define BEACON_SSID_OFFSET 38
#define SRCADDR_OFFSET 10
#define BSSID_OFFSET 16
#define SEQNUM_OFFSET 22


int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
    return 0;
}

int wifi_stop_dhcp_server(esp_netif_t *netif);

int wifi_set_dns_server(esp_netif_t *netif);

int wifi_start();

int wifi_stop();

int wifi_set_default_config();

void wifi_void_set_default_config();

int wifi_disconnect(uint8_t* bssid, char* ssid, uint8_t channel, wifi_auth_mode_t auth_mode,  unsigned int seconds);
#endif // WIFI_CONTROLLER_H