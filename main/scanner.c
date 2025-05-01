
#include "scanner.h"
void print_ap_info(wifi_ap_record_t* ap) {
    printf("SSID: %s\n", ap->ssid);
    printf("Primary Channel: %u\n", ap->primary);
    printf("BSSID (MAC Address): %02X:%02X:%02X:%02X:%02X:%02X\n",
           ap->bssid[0], ap->bssid[1], ap->bssid[2],
           ap->bssid[3], ap->bssid[4], ap->bssid[5]);
}

void print_ap_records(wifictl_ap_records_t* ap_records){
    unsigned i;
    for(i = 0; i < ap_records->count; ++i){
        wifi_ap_record_t record = ap_records->records[i];
        print_ap_info(&record);
    }
}

int scan_networks(wifictl_ap_records_t* ap_records)
{
    ap_records->count = CONFIG_SCAN_MAX_AP;
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE
    };
    
    int ret = esp_wifi_scan_start(&scan_config, true);
    if(ret != ESP_OK) {
        return -1;
    }
    ret =  esp_wifi_scan_get_ap_records(&ap_records->count, ap_records->records);
    if(ret != ESP_OK) {
        return -1;
    }
    return 0;
}

char* records_to_json(wifictl_ap_records_t* ap){
    char* json = malloc(2048);
    if(json == NULL){
        return NULL;
    }
    int offset = 0;
    offset += snprintf(json + offset, 2048 - offset, "[");
    for(int i = 0; i < ap->count; i++){
        wifi_ap_record_t record = ap->records[i];
        offset += snprintf(json + offset, 2048 - offset, "{ \"ssid\": \"%s\", \"bssid\": \"%02X:%02X:%02X:%02X:%02X:%02X\" }",
                           record.ssid,
                           record.bssid[0], record.bssid[1], record.bssid[2],
                           record.bssid[3], record.bssid[4], record.bssid[5]);
        if(i < ap->count - 1){
            offset += snprintf(json + offset, 2048 - offset, ",");
        }
    }
    offset += snprintf(json + offset, 2048 - offset, "]");
    json[offset] = '\0';
    return json;
}