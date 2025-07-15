#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include "driver/gpio.h" // For use of hardware GPIO's
#include "driver/ledc.h" // For generating pwm output signals
#include "freertos/queue.h" // For using freertos queues

#include "driver/i2c_slave.h"
#include "PWM_write.h"

#define I2C_SLAVE_PORT       0
#define I2C_SCL_IO           GPIO_NUM_22
#define I2C_SDA_IO           GPIO_NUM_21
#define I2C_FREQ_HZ          100000
#define I2C_DEVICE_ADDR      0x68
#define ESP_SLAVE_ADDR 0x0A

#define PWM_OUTPUT_GPIO GPIO_NUM_21

static const char *TAG = "I2C_SLAVE";

QueueHandle_t pwmOutputQueue; // Queue for passing pwm signal from pid to pwm output

int32_t pulse_width_out_us = 0;
int PWM_output_period_ms = 10;
static TimerHandle_t pwmOutTimer;

i2c_slave_read_buffer(I2C_SLAVE_PORT, data, size, timeout);
i2c_slave_write_buffer(I2C_SLAVE_PORT, response, size, timeout);


void slave_task(void *arg) {
    uint8_t buffer[128];
    while (1) {
        int size = i2c_slave_read_buffer(I2C_SLAVE_PORT, buffer, sizeof(buffer), pdMS_TO_TICKS(1000));
        if (size > 0) {
            ESP_LOGI(TAG, "Received %d bytes: 0x%02x", size, buffer[0]);
            // Optionally write back a response
            uint8_t reply = 0x55;
            i2c_slave_write_buffer(I2C_SLAVE_PORT, &reply, 1, pdMS_TO_TICKS(100));
        }
    }
}



void setup(void)
{
    printf("Initializing I2C master...\n");

    i2c_slave_config_t slave_config = {
        .addr_10bit_en = 0,
        .slave_addr = ESP_SLAVE_ADDR,
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .maximum_speed = I2C_FREQ_HZ,
    };


    ESP_ERROR_CHECK(i2c_slave_driver_initialize(I2C_SLAVE_PORT, &slave_config));

    device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device_config.device_address = I2C_DEVICE_ADDR;
    device_config.scl_speed_hz = I2C_FREQ_HZ;

    //PWM initialize
    pwmOutputQueue = xQueueCreate(5, sizeof(int32_t));// Create a queue for pwm output values
    pwm_set_receive_queue(pwmOutputQueue); // Set queue to receive pwm value
    PWM_output_config(PWM_OUTPUT_GPIO); // Setup PWM output

    // Create periodic timer to output new PWM value to motor
    pwmOutTimer = xTimerCreate("PWM OUTPUT", pdMS_TO_TICKS(PWM_output_period_ms), pdTRUE, NULL, PWM_output_update);
    if (pwmOutTimer == NULL || xTimerStart(pwmOutTimer, 0) != pdPASS) {
        ESP_LOGE(TAG_PWM_LEDC, "Failed to create/start PWM OUTPUT timer");
    }
}

void app_main(void)
{
    setup();
    xTaskCreate(slave_task, "slave-revice-task", 1024, NULL, 1, NULL);

}
