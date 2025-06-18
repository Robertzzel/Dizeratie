#ifndef OS_FUNCTIONS_H
#define OS_FUNCTIONS_H
#include "esp_timer.h"
#include <errno.h>
#include <limits.h>
int parse_int(const char* str, int* out_value);
int delay_function(esp_timer_cb_t callback, unsigned int seconds);
#endif 