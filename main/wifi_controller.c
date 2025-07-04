#include "wifi_controller.h"
int wifi_stop_dhcp_server(esp_netif_t *netif){
    int ret = esp_netif_dhcps_stop(netif);
    if (ret != ESP_OK) {
        return ret;
    }

    esp_netif_dhcp_status_t dhcpc_status;
    ret = esp_netif_dhcps_get_status(netif, &dhcpc_status);
    if (ret != ESP_OK) {
        return ret;
    }
    if (dhcpc_status != ESP_NETIF_DHCP_STOPPED) {
        return -1;
    }
    return ESP_OK;
}

int wifi_set_dns_server(esp_netif_t *netif)
{
    // stop dhcp client
    int ret = wifi_stop_dhcp_server(netif);
    if (ret != ESP_OK) {
        ESP_LOGE("WIFICONTROLLER", "Failed to stop DHCP client: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr("192.168.10.1");
    ip.netmask.addr = ipaddr_addr("255.255.255.0");
    ip.gw.addr = ipaddr_addr("192.168.10.1");

    ret = esp_netif_set_ip_info(netif, &ip);
    if (ret != ESP_OK) {
        ESP_LOGE("WIFICONTROLLER", "Failed to set IP info: %s", esp_err_to_name(ret));
        return -1;
    }

    uint32_t addr = ipaddr_addr("192.168.10.1");
    if (!addr || (addr == IPADDR_NONE)) {
        ESP_LOGE("WIFICONTROLLER", "Invalid DNS server address");
        return -1;
    }
    
    esp_netif_dns_info_t dns;
    dns.ip.u_addr.ip4.addr = addr;
    dns.ip.type = IPADDR_TYPE_V4;

    ret = esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns);
    if (ret != ESP_OK) {
        ESP_LOGE("WIFICONTROLLER", "Failed to set DNS server");
        return ret;
    }
    ret = esp_netif_set_dns_info(netif, ESP_NETIF_DNS_BACKUP, &dns);
    if (ret != ESP_OK) {
        ESP_LOGE("WIFICONTROLLER", "Failed to set backup DNS server");
        return ret;
    }
    return esp_netif_dhcps_start(netif);
}

int wifi_start() 
{
    int ret = esp_netif_init();
    if(ret != ESP_OK)
    {
        return ret;
    }

    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
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
    ret = wifi_set_dns_server(ap_netif);
    if(ret != ESP_OK)
    {
        return ret;
    }
    return ESP_OK;
}

int wifi_stop()
{
    wifi_config_t wifi_config = 
    {
        .ap = 
        {
            .max_connection = 0
        },
    };
    return esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
}

int wifi_set_default_config()
{
    wifi_config_t config = 
    {
        .ap = 
        {
            .ssid = "ESP32",
            .ssid_len = strlen("ESP32"),
            .channel = 1,
            .authmode = WIFI_AUTH_OPEN,
            .password = "ESP32ESP32",
            .max_connection = 2
        },
    };
    esp_wifi_set_mac(WIFI_IF_AP, original_mac_ap);
    return esp_wifi_set_config(ESP_IF_WIFI_AP, &config);
}

void wifi_void_set_default_config()
{
    ESP_LOGE("WIFICONTROLLER", "Setting default config");
    wifi_set_default_config();
}

int wifi_disconnect(uint8_t* bssid, char* ssid, uint8_t channel, wifi_auth_mode_t auth_mode,  unsigned int seconds)
{
    delay_function(&wifi_void_set_default_config, seconds);

    int ret = esp_wifi_set_mac(WIFI_IF_AP, bssid);
    if(ret != ESP_OK) 
    {
        ESP_LOGE("WIFICONTROLLER", "Failed to set MAC address");
        return ret;
    }
    wifi_config_t config = 
    {
        .ap = 
        {
            .ssid_len = strlen(ssid),
            .channel = channel,
            .authmode = auth_mode,
            .password = "ESP32ESP32",
            .max_connection = 1
        },
    };
    memcpy(config.ap.ssid, ssid, strlen(ssid));
    return esp_wifi_set_config(ESP_IF_WIFI_AP, &config);
}