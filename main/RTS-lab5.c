#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/queue.h"
#include "driver/i2c.h"
#include "driver/i2c_slave.h"
#include "PWM_write.h"

#define I2C_SLAVE_PORT       I2C_NUM_0
#define I2C_SCL_IO           GPIO_NUM_22
#define I2C_SDA_IO           GPIO_NUM_21
#define I2C_FREQ_HZ          100000
#define ESP_SLAVE_ADDR       0x0A
#define PWM_OUTPUT_GPIO      GPIO_NUM_21

static const char *TAG = "I2C_SLAVE";

QueueHandle_t pwmOutputQueue;


int32_t pulse_width_out_us = 0;
int PWM_output_period_ms = 10;
static TimerHandle_t pwmOutTimer;

void slave_task(void *arg) {
    uint8_t buffer[128];
    while (1) {
        int size = i2c_slave_read_buffer(I2C_SLAVE_PORT, buffer, sizeof(buffer), pdMS_TO_TICKS(1000));
        if (size > 0) {
            ESP_LOGI(TAG, "Received %d bytes: 0x%02x", size, buffer[0]);
            uint8_t reply = 0x55;
            i2c_slave_write_buffer(I2C_SLAVE_PORT, &reply, 1, pdMS_TO_TICKS(100));
        }
        xQueueSend(pwmOutputQueue, buffer, portMAX_DELAY);
    }
}

void setup(void) {
    printf("Initializing I2C slave...\n");

    // Configure I2C controller as slave
    i2c_config_t conf_slave = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = ESP_SLAVE_ADDR,
    };
    
    ESP_ERROR_CHECK(i2c_param_config(I2C_SLAVE_PORT, &conf_slave));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_SLAVE_PORT, conf_slave.mode, 0, 0, 0));

    // PWM initialize
    pwmOutputQueue = xQueueCreate(5, sizeof(int32_t));
    pwm_set_receive_queue(pwmOutputQueue);
    PWM_output_config(PWM_OUTPUT_GPIO);

    // Create periodic timer for PWM output
    pwmOutTimer = xTimerCreate("PWM OUTPUT", pdMS_TO_TICKS(PWM_output_period_ms), pdTRUE, NULL, PWM_output_update);
    if (pwmOutTimer == NULL || xTimerStart(pwmOutTimer, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create/start PWM OUTPUT timer");
    }
}

void app_main(void) {
    setup();
    xTaskCreate(slave_task, "slave-receive-task", 1024, NULL, 1, NULL);
}