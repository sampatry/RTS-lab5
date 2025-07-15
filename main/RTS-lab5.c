#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <stdio.h>

#define I2C_MASTER_PORT      0
#define I2C_SCL_IO           22
#define I2C_SDA_IO           21
#define I2C_FREQ_HZ          100000
#define I2C_DEVICE_ADDR      0x68

#define ESP_SLAVE_ADDR 0x0A

static const char *TAG = "I2C_MASTER";

// Global handles so both setup() and app_main() can use them
static i2c_master_bus_handle_t i2c_bus_handle;
static i2c_device_config_t device_config;
static i2c_master_dev_handle_t dev_handle;

void task1(void *arg) {

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &device_config, &dev_handle));
    
    while (1) {
        // Write register address to read from
        uint8_t reg_addr = ESP_SLAVE_ADDR;
        ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, &reg_addr, 1, pdMS_TO_TICKS(1000)));

        // Read 1 byte from the device
        uint8_t data;
        ESP_ERROR_CHECK(i2c_master_receive(dev_handle, &data, 1, pdMS_TO_TICKS(1000)));

        ESP_LOGI(TAG, "Received byte: 0x%02x", data);

        vTaskDelay(pdMS_TO_TICKS(2000));  // wait 2 seconds before reading again
    }
}


void setup(void)
{
    printf("Initializing I2C master...\n");

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .glitch_ignore_cnt = 7
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device_config.device_address = I2C_DEVICE_ADDR;
    device_config.scl_speed_hz = I2C_FREQ_HZ;
}

void app_main(void)
{
    setup();

    xTaskCreate(task1, "master-send-task", 1024, NULL, 1, NULL);

    // Cleanup if ever reached (usually app_main runs forever)
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    ESP_ERROR_CHECK(i2c_del_master_bus(i2c_bus_handle));
}
