#ifndef __UART_TERMINAL_H
#define __UART_TERMINAL_H

#include <stdio.h>
#include <inttypes.h>
#include "driver/uart.h"
#include "string.h"
#include "esp_err.h"


#define UART_NUM UART_NUM_1     // uart num
#define BUF_SIZE 1024            // buffer size
#define CMD_LINE_MAX  5

#define PARAMETER_NUM 4

#define DEVICE_VALUE "0x73"
#define REG_VALUE "0x00042034"
#define WRITE "1"
#define WDATA "0x0"


typedef struct 
{
    char *option;
    char *value;
}parameter_t;

typedef struct 
{
    uint8_t len;
    char *input;
    parameter_t parameter[PARAMETER_NUM];
}cmd_t;

typedef struct 
{
    uint16_t cmd_num;

    cmd_t cmd[CMD_LINE_MAX];
}cmdline_t;


extern cmdline_t cmdline;

/**
 * @brief exit cmd
 */
// void cmd_exit(void);

/**
 * @brief get value
 *
 * @param option cmd option
 * @param value option -> value
 *
 * @return
 *     - ESP_OK   Success
 *     - ESP_FAIL Error
 */
esp_err_t cmd_get_value(cmd_t *cmd, const char *option, char **value);

/**
 * @brief start cmd
 */
void cmd_start(void);

/**
 * @brief init cmd
 */
void cmd_init(void);

void cmd_exit(void);


#endif
