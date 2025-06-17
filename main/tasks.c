#include "tasks.h"

void flood_task(void* pvParameters) {
    flood_running = true;

    uint8_t line_index = 0;
    uint8_t current_beacon[200];
    uint16_t seq[TOTAL_LINES] = { 0 };

    while (flood_running) {
        vTaskDelay(100 / TOTAL_LINES / portTICK_PERIOD_MS);

        memcpy(current_beacon, beacon_frame, BEACON_SSID_OFFSET - 1);

        char* ssid = ssids[line_index];
        uint8_t ssid_len = strlen(ssid);

        current_beacon[BEACON_SSID_OFFSET - 1] = ssid_len;
        memcpy(current_beacon + BEACON_SSID_OFFSET, ssid, ssid_len);
        memcpy(current_beacon + BEACON_SSID_OFFSET + ssid_len, beacon_frame + BEACON_SSID_OFFSET, sizeof(beacon_frame) - BEACON_SSID_OFFSET);

        current_beacon[SRCADDR_OFFSET + 5] = line_index;
        current_beacon[BSSID_OFFSET + 5] = line_index;

        current_beacon[SEQNUM_OFFSET] = (seq[line_index] & 0x0f) << 4;
        current_beacon[SEQNUM_OFFSET + 1] = (seq[line_index] & 0xff0) >> 4;

        seq[line_index]++;
        if (seq[line_index] > 0xfff)
            seq[line_index] = 0;

        esp_wifi_80211_tx(WIFI_IF_AP, current_beacon, sizeof(beacon_frame) + ssid_len, false);

        if (++line_index >= TOTAL_LINES)
            line_index = 0;
    }

    flood_task_handle = NULL;
    vTaskDelete(NULL);
}