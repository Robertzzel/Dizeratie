#ifndef TASKS_H
#define TASKS_H

volatile bool flood_running = false;
TaskHandle_t flood_task_handle = NULL;

void flood_task(void* pvParameters) {
    flood_running = true;

    uint8_t line_index = 0;
    uint8_t beacon_rick[200];
    uint16_t seq[TOTAL_LINES] = { 0 };

    while (flood_running) {
        vTaskDelay(100 / TOTAL_LINES / portTICK_PERIOD_MS);

        memcpy(beacon_rick, beacon_frame, BEACON_SSID_OFFSET - 1);

        char* ssid = rick_ssids[line_index];
        uint8_t ssid_len = strlen(ssid);

        printf("Sending frame %d for %s\n",line_index, ssid);
        beacon_rick[BEACON_SSID_OFFSET - 1] = ssid_len;
        memcpy(beacon_rick + BEACON_SSID_OFFSET, ssid, ssid_len);
        memcpy(beacon_rick + BEACON_SSID_OFFSET + ssid_len, beacon_frame + BEACON_SSID_OFFSET, sizeof(beacon_frame) - BEACON_SSID_OFFSET);

        beacon_rick[SRCADDR_OFFSET + 5] = line_index;
        beacon_rick[BSSID_OFFSET + 5] = line_index;

        beacon_rick[SEQNUM_OFFSET] = (seq[line_index] & 0x0f) << 4;
        beacon_rick[SEQNUM_OFFSET + 1] = (seq[line_index] & 0xff0) >> 4;

        seq[line_index]++;
        if (seq[line_index] > 0xfff)
            seq[line_index] = 0;

        esp_wifi_80211_tx(WIFI_IF_AP, beacon_rick, sizeof(beacon_frame) + ssid_len, false);

        if (++line_index >= TOTAL_LINES)
            line_index = 0;
    }

    printf("while broke %d\n", flood_running);
    flood_task_handle = NULL;
    vTaskDelete(NULL);
}
#endif // TASKS_H