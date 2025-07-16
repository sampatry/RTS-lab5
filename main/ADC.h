#ifndef ADC_H
#define ADC_H

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "esp_log.h"

extern const char *TAG_ADC;

void adc_init();
void adc_get(TimerHandle_t xTimer);

#endif