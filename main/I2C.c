#include "I2C.h"

static const char *TAG_I2C = "I2C_MASTER";
static QueueHandle_t i2c_queue = NULL;
static i2c_master_bus_handle_t i2c_bus_handle;
static i2c_master_dev_handle_t dev_handle;

void i2c_init(QueueHandle_t queue){
    ESP_LOGI(TAG_I2C, "Initializing I2C master...");
    i2c_queue = queue;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .scl_io_num = I2C_SCL_PIN,
        .sda_io_num = I2C_SDA_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = I2C_DEVICE_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &device_config, &dev_handle));
}

void I2C_send(void *arg) {
    int data;
    // Write register address to read from
    uint8_t reg_addr = ESP_SLAVE_ADDR;
    while (1) {
        if (xQueueReceive(i2c_queue, &data, portMAX_DELAY) == pdPASS){
            ESP_LOGI(TAG_I2C, "ADC data: %d", data);
            ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, data, sizeof(data), -1));
        }
    }
}