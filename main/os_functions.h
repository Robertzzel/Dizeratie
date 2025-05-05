#ifndef OS_FUNCTIONS_H
#define OS_FUNCTIONS_H

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

#endif // OS_FUNCTIONS_H