#ifndef I2C_H
#define I2C_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"


#define I2C_MASTER_PORT      0
#define I2C_SCL_PIN           GPIO_NUM_22
#define I2C_SDA_PIN           GPIO_NUM_21
#define I2C_FREQ_HZ          100000
#define I2C_DEVICE_ADDR      0x68

#define ESP_SLAVE_ADDR 0x0A

void i2c_init(QueueHandle_t queue);
void I2C_send(void *arg);

#endif