#ifndef I2C_H
#define I2C_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"


#define I2C_MASTER_PORT 0
#define I2C_SDA_PIN     GPIO_NUM_21
#define I2C_SCL_PIN     GPIO_NUM_22
#define I2C_FREQ_HZ     100000
#define SLAVE_ADDR      0x42

#define TAG_I2C "I2C_MASTER"

void i2c_master_init(QueueHandle_t queue);
void I2C_send(void *arg);

#endif