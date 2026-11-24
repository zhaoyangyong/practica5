#include "sharp.h"
#include "sdkconfig.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

ESP_EVENT_DEFINE_BASE(SHARP_EVENT);
static const char *TAG = "sharp";
static adc_oneshot_unit_handle_t adc;
static adc_cali_handle_t calibration;
static esp_timer_handle_t timer;
static portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
static float latest_cm;

static esp_err_t measure(void)
{
    int total_mv = 0;
    for (int i = 0; i < CONFIG_SHARP_SAMPLES; ++i) {
        int raw, mv;
        ESP_RETURN_ON_ERROR(adc_oneshot_read(adc, ADC_CHANNEL_6, &raw), TAG, "lectura ADC");
        ESP_RETURN_ON_ERROR(adc_cali_raw_to_voltage(calibration, raw, &mv), TAG, "calibracion");
        total_mv += mv;
    }
    // Aproximacion inversa a partir de 1580 mV a 8 cm y 600 mV a 22 cm.
    float mean_mv = (float)total_mv / CONFIG_SHARP_SAMPLES;
    float cm = mean_mv > 40 ? 12320.0f / (mean_mv - 40.0f) : 0;
    if (cm < 4 || cm > 30) {
        ESP_LOGW(TAG, "%.0f mV fuera de la curva de 4-30 cm", mean_mv);
        return ESP_OK;
    }
    portENTER_CRITICAL(&lock);
    latest_cm = cm;
    portEXIT_CRITICAL(&lock);
    return esp_event_post(SHARP_EVENT, SHARP_READY, NULL, 0, 0);
}

static void tick(void *arg)
{
    esp_err_t err = measure();
    if (err != ESP_OK) ESP_LOGE(TAG, "Medida: %s", esp_err_to_name(err));
}

esp_err_t sharp_init(void)
{
    if (timer) return ESP_ERR_INVALID_STATE;
    adc_oneshot_unit_init_cfg_t unit = { .unit_id = ADC_UNIT_1 };
    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&unit, &adc), TAG, "ADC1");
    adc_oneshot_chan_cfg_t channel = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    esp_err_t err = adc_oneshot_config_channel(adc, ADC_CHANNEL_6, &channel);
    if (err == ESP_OK) {
        adc_cali_line_fitting_config_t cfg = { .unit_id = ADC_UNIT_1, .atten = channel.atten,
                                               .bitwidth = channel.bitwidth };
        err = adc_cali_create_scheme_line_fitting(&cfg, &calibration);
    }
    if (err == ESP_OK) {
        esp_timer_create_args_t cfg = { .callback = tick, .name = "sharp" };
        err = esp_timer_create(&cfg, &timer);
        if (err != ESP_OK) ESP_ERROR_CHECK_WITHOUT_ABORT(adc_cali_delete_scheme_line_fitting(calibration));
    }
    if (err != ESP_OK) ESP_ERROR_CHECK_WITHOUT_ABORT(adc_oneshot_del_unit(adc));
    return err;
}

esp_err_t sharp_start(void) { return esp_timer_start_periodic(timer, 1000000); }
esp_err_t sharp_stop(void) { return esp_timer_stop(timer); }

esp_err_t sharp_get_distance(float *cm)
{
    if (!cm) return ESP_ERR_INVALID_ARG;
    portENTER_CRITICAL(&lock);
    *cm = latest_cm;
    portEXIT_CRITICAL(&lock);
    return *cm > 0 ? ESP_OK : ESP_ERR_INVALID_STATE;
}
