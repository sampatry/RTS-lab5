#include "ADC.h"

static QueueHandle_t adc_queue = NULL;

void adc_init(QueueHandle_t queue) {
    ESP_LOGI(TAG_I2C, "Initializing ADC...");
    adc_queue = queue;
    adc1_config_width(ADC_WIDTH_BIT_12);   // 12-bit width (0-4095)
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO34, 11dB for 3.3V which is max for esp32
}

void adc_get(TimerHandle_t xTimer) {
    while (1) {
        int val = adc1_get_raw(ADC1_CHANNEL_6);
        ESP_LOGI(TAG_ADC, "Raw ADC Value: %d", val);
        xQueueSend(adc_queue, &val, 0); //sends raw IMU data
    }
}


