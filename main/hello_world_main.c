/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "driver/i2c.h"
#include "uart_terminal.h"
#include "esp_log.h"

static const char *TAG = "i2c-simple-example";

#define I2C_MASTER_SCL_IO           19                          /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           18                          /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                           /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000                      /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define SWITCH_ADDR                 0x73                        /*!< Slave address of the MPU9250 sensor */
#define SWITCH_TEST_REG             0x00042034                  /*!< Register addresses of the "who am I" register */

char *defult_parameter_option_2[PARAMETER_NUM] = {"device",  "reg", "wdata", "w"};
cmdline_t cmdline;
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t i2c_write(i2c_port_t i2c_num, uint8_t device_address, uint32_t reg_address, uint32_t reg_data)
{
    uint8_t data[8];
    data[0] = reg_address >> 24; 
    data[1] = reg_address >> 16; 
    data[2] = reg_address >> 8; 
    data[3] = reg_address; 

    data[4] = reg_data >> 24; 
    data[5] = reg_data >> 16; 
    data[6] = reg_data >> 8; 
    data[7] = reg_data; 
    return i2c_master_write_to_device(i2c_num, device_address, data, sizeof(data), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t i2c_read(i2c_port_t i2c_num, uint8_t device_address, uint32_t reg_address, uint8_t* read_buffer)
{
    esp_err_t err;
    uint8_t data[4];
    data[0] = reg_address >> 24; 
    data[1] = reg_address >> 16; 
    data[2] = reg_address >> 8; 
    data[3] = reg_address; 

    err = i2c_master_write_to_device(i2c_num, device_address, data, 4, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    err = i2c_master_read_from_device(i2c_num, device_address, read_buffer, 4, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return err;
}

// 将32位整数从小端字节序转换为大端字节序
uint32_t swap_endian_32(uint32_t value) {
    return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) |
           ((value >> 8) & 0xFF00) | ((value >> 24) & 0xFF);
}

void app_main(void)
{
    printf("Hello world!\n");
    uint8_t *rdata = (uint8_t *)malloc(4);
    uint32_t device_addr = 0, reg_addr = 0, write = 0, wdata = 0;

    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    i2c_read(I2C_MASTER_NUM, SWITCH_ADDR, SWITCH_TEST_REG, rdata);
    // 检查寄存器的默认值
    assert(swap_endian_32((*(uint32_t *)rdata)) == 0x000088a8);
    *rdata = 0;
    cmd_init();
    while(1) {
        cmd_start();
        for (int j = 0; j < cmdline.cmd_num; j++) {
            for (int i = 0; i < PARAMETER_NUM; i++) {
                char *value;
                ESP_ERROR_CHECK(cmd_get_value(&cmdline.cmd[j], defult_parameter_option_2[i], &value));
                // printf("[%s]: %s    \n", defult_parameter_option_2[i], value);
                switch (i)
                {
                    case 0:
                        device_addr = strtol((char *)value, NULL, 16);
                        continue;
                    case 1:
                        reg_addr = strtol((char *)value, NULL, 16);
                        continue;            
                    case 2:
                        wdata = strtol((char *)value, NULL, 16);
                        continue;  
                    case 3:
                        write = atoi(value);
                        continue;           
                    default:
                        break;
                }
            }
            //printf("[device_addr]: %lx  [reg_addr]: %lx [write]: %lx  [wdata]: %lx  \n", device_addr, reg_addr, write, wdata);
            if (write == 1) {
                i2c_write(I2C_MASTER_NUM, device_addr, reg_addr, wdata);
            }else {
                i2c_read(I2C_MASTER_NUM, device_addr, reg_addr, rdata);

                if(((((reg_addr>>12)&0xFF) == 0x62 || ((reg_addr>>12)&0xFF) == 0x64 || ((reg_addr>>12)&0xFF) == 0x66 || ((reg_addr>>12)&0xFF) == 0x68 || ((reg_addr>>12)&0xFF) == 0x6a)) && (((reg_addr&0xff) == 0x40) || ((reg_addr&0xff) == 0x44)))
                {   
                    // in these case ,read data is active in bit [31:16]
                    //printf("device_addr: 0x%lx   reg_addr: 0x%08lx   rdata: 0x%04lx\n", (uint32_t)device_addr, (uint32_t)reg_addr, (swap_endian_32(*(uint32_t *)rdata))>>16);
                    printf("reg_addr: 0x%08lx   rdata: 0x%04lx\n", (uint32_t)reg_addr, (swap_endian_32(*(uint32_t *)rdata))>>16);
                }
                else
                {
                    printf("device_addr: 0x%lx   reg_addr: 0x%08lx   rdata: 0x%08lx\n", (uint32_t)device_addr, (uint32_t)reg_addr, swap_endian_32(*(uint32_t *)rdata));
                }
                
                //printf("device_addr: 0x%lx   reg_addr: 0x%08lx   rdata: 0x%08lx\n", (uint32_t)device_addr, (uint32_t)reg_addr, swap_endian_32(*(uint32_t *)rdata));
                *rdata = 0;
            }
        }
        // printf("%d\n", cmdline.cmd_num);

        cmd_exit();
    }

    free(rdata);
}
