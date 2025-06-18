#include "os_functions.h"

int parse_int(const char* str, int* out_value) {
    char* endptr;
    errno = 0;

    long val = strtol(str, &endptr, 10);

    // Check for various possible errors
    if (errno == ERANGE || val > INT_MAX || val < 1) {
        return 0; // Out of range
    }

    if (endptr == str || *endptr != '\0') {
        return 0; // No digits were found or junk after number
    }

    *out_value = (int)val;
    return 1; // Success
}

int delay_function(esp_timer_cb_t callback, unsigned int seconds){
    const esp_timer_create_args_t my_timer_args = {
        .callback = callback,
        .arg = NULL,
        .name = "my_timer"
    };

    esp_timer_handle_t my_timer;
    esp_err_t err = esp_timer_create(&my_timer_args, &my_timer);
    if (err != ESP_OK) {
        return -1;
    }

    esp_timer_start_once(my_timer, seconds * 1000000);
    return 0;
}