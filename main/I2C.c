#include "I2C.h"

static QueueHandle_t i2c_queue = NULL;
i2c_master_dev_handle_t slave_dev;
i2c_master_bus_handle_t bus_handle;

void i2c_master_init(QueueHandle_t queue){
    ESP_LOGI(TAG_I2C, "Initializing I2C master...");
    i2c_queue = queue;

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SLAVE_ADDR,
        .scl_speed_hz = I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &slave_dev));
}

void I2C_send(void *arg) {
    uint8_t data[2];
    uint16_t raw_adc;
    while (1) {       
        if (xQueueReceive(i2c_queue, &raw_adc, portMAX_DELAY) == pdPASS){
            data[0] = (raw_adc >> 8) & 0xFF;
            data[1] = raw_adc & 0xFF;
            ESP_LOGI(TAG_I2C, "Send data: %d %d", data[1], data[0]);

            if (i2c_master_transmit(slave_dev, data, 2, pdMS_TO_TICKS(100)) == ESP_OK) {
                ESP_LOGI(TAG_I2C, "Sent value: %d", raw_adc);
            } else {
                ESP_LOGE(TAG_I2C, "Transmission failed");
            }
        }
    }
}