#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"

void adc_init() {
    adc1_config_width(ADC_WIDTH_BIT_12);   // 12-bit width (0-4095)
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO34, 11dB for 3.3V which is max for esp32
}

void adc_task(void *pvParameter) {
    while (1) {
        int val = adc1_get_raw(ADC1_CHANNEL_6);
        ESP_LOGI("ADC", "Raw ADC Value: %d", val);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
    }
}

void app_main(void) {
    adc_init();
    xTaskCreate(adc_task, "ADC Task", 2048, NULL, 5, NULL);
}
