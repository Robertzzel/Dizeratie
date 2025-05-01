#ifndef STATE_H
#define STATE_H

typedef struct
{
    bool event_loop_started;
    bool nvs_flash_initialized;
    bool wifi_started;
} state_t;

// int state_star_event_loop(state_t* state)
// {
//     int ret;
//     if (!state->event_loop_started)
//     {
//         ret = esp_event_loop_create_default();
//         state->event_loop_started = true;
//     }
//     return ret;
// }

// int state_init_nvs_flash(state_t* state)
// {
//     int ret;
//     if (!state->nvs_flash_initialized)
//     {
//         ret = nvs_flash_init();
//         state->nvs_flash_initialized = true;
//     }
//     return ret;
// }
#endif // STATE_H