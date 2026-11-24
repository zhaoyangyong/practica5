#include "sharp.h"
#include "esp_log.h"

static void show_distance(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    float cm;
    if (ESP_ERROR_CHECK_WITHOUT_ABORT(sharp_get_distance(&cm)) == ESP_OK)
        ESP_LOGI("practica5", "Distancia: %.1f cm", cm);
}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(sharp_init());
    ESP_ERROR_CHECK(esp_event_handler_register(SHARP_EVENT, SHARP_READY, show_distance, NULL));
    ESP_ERROR_CHECK(sharp_start());
}
