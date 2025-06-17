#ifndef TASKS_H
#define TASKS_H

#include <string.h>
#include "wifi_controller.h"

volatile bool flood_running = false;
TaskHandle_t flood_task_handle = NULL;

char *ssids[] = {
	"1 Acesta",
	"2 Este",
	"3 Un",
	"4 Test",
	"5 De",
	"6 Retea",
    "7 Wi-Fi",
};
#define TOTAL_LINES (sizeof(ssids) / sizeof(char *))

void flood_task(void* pvParameters);
#endif // TASKS_H