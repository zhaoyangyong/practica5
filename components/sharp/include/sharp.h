#pragma once
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(SHARP_EVENT);
enum { SHARP_READY };
esp_err_t sharp_init(void);
esp_err_t sharp_start(void);
esp_err_t sharp_stop(void);
esp_err_t sharp_get_distance(float *cm);
