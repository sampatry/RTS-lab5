#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "PWM_write.h"

#define I2C_SLAVE_NUM        I2C_NUM_0
#define I2C_SCL_IO           GPIO_NUM_22
#define I2C_SDA_IO           GPIO_NUM_21
#define I2C_FREQ_HZ          100000
#define I2C_SLAVE_ADDR       0x0A
#define PWM_OUTPUT_GPIO      GPIO_NUM_26

static const char *TAG = "I2C_SLAVE";

QueueHandle_t pwmOutputQueue;
static TimerHandle_t pwmOutTimer;

int32_t pulse_width_out_us = 0;
int PWM_output_period_ms = 10;

i2c_slave_dev_handle_t slave_handle;

void i2c_slave_task(void *arg)
{
    uint8_t data[128];
    
    while (1) {
        // Receive data (this function blocks until data is received)
        esp_err_t ret = i2c_slave_receive(slave_handle, data, sizeof(data));
        
        if (ret == ESP_OK) {

            // For your 4-byte PWM value case:
            ESP_LOGI(TAG, "Received data: 0x%02x 0x%02x 0x%02x 0x%02x", data[0], data[1], data[2], data[3]);
            
            pulse_width_out_us = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
            xQueueSend(pwmOutputQueue, &pulse_width_out_us, portMAX_DELAY);
            
            // Example response
            uint8_t reply = 0x55;
            i2c_slave_transmit(slave_handle, &reply, 1, pdMS_TO_TICKS(100));
        } else {
            ESP_LOGE(TAG, "I2C receive error: %s", esp_err_to_name(ret));
            vTaskDelay(pdMS_TO_TICKS(100)); // Delay to prevent busy loop
        }
    }
}

void setup(void)
{
    printf("Initializing I2C slave...\n");

    // Configure I2C slave
    i2c_slave_config_t i2c_slv_config = {
        .i2c_port = I2C_SLAVE_NUM,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .slave_addr = I2C_SLAVE_ADDR,
        .send_buf_depth = 100,
        //.receive_buf_depth = 100,
    };

    ESP_ERROR_CHECK(i2c_new_slave_device(&i2c_slv_config, &slave_handle));
    //i2c_new_slave_device(&i2c_slv_config, &slave_handle);

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

void app_main(void)
{
    setup();
    xTaskCreate(i2c_slave_task, "i2c_slave_task", 4096, NULL, 5, NULL);
}