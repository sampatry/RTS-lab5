#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "ADC.h"
#include "I2C.h"

QueueHandle_t data_Queue; // Queue for ADC data
static TimerHandle_t ADC_Timer; // Timer handle for calling adc get function

void app_main(void)
{
    data_Queue = xQueueCreate(5, sizeof(uint16_t)); // Create queue to store up to 5 values of ADC data

    i2c_master_init(data_Queue); // Setup I2C master, and data queue
    adc_init(data_Queue); // Setup ADC GPIO, voltage range, bits and data queue

    vTaskDelay(pdMS_TO_TICKS(1000)); // Small startup delay after initialization

    xTaskCreate(I2C_send, "master-send-task", 2048, NULL, 1, NULL);

    // Start ADC read timer
    ADC_Timer = xTimerCreate("Pot ADC", pdMS_TO_TICKS(1000), pdTRUE, NULL, adc_get);
    if (ADC_Timer == NULL || xTimerStart(ADC_Timer, 0) != pdPASS) {
        ESP_LOGE(TAG_ADC, "Failed to create/start ADC timer");
    }
}