/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "uart_terminal.h"

// static const char *TAG = "UART TEST";

char *defult_parameter_option[PARAMETER_NUM] = {"device",  "reg", "wdata", "w"};
char *defult_parameter_value[PARAMETER_NUM]  = {DEVICE_VALUE, REG_VALUE, WDATA, WRITE};

// cmd_t cmd;
void DebugUartInit(void)
{
    uart_config_t uartConfig = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,	
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
	};

	uart_driver_install(UART_NUM_0, BUF_SIZE, BUF_SIZE * 2, 0, NULL, 0);
	uart_param_config(UART_NUM_0, &uartConfig);
	uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void cmd_register_parameter(void){
    // printf (" input format defult:");
    for (int j = 0; j < CMD_LINE_MAX; j++){
        for (int i = 0; i < PARAMETER_NUM; i++) {
            cmdline.cmd[j].parameter[i].option = defult_parameter_option[i];
        }
    }
    // printf("\n");
}

void cmd_decode(cmd_t *cmd) 
{
    // split.('-')
    uint8_t begin[PARAMETER_NUM] = {0};  
    uint8_t input_len[PARAMETER_NUM] = {0};
    uint8_t option_begin[PARAMETER_NUM] = {0};     // left closed right open
    uint8_t option_len[PARAMETER_NUM] = {0};
    uint8_t value_begin[PARAMETER_NUM] = {0};  
    uint8_t value_len[PARAMETER_NUM] = {0};

    // printf("decode: cmd.input: %s\n", cmd->input);
    // printf("decode: cmd.len: %d\n", cmd->len);

    int option_num = 0;
    for (int i = 0; i < cmd->len; i++) {
        if (cmd->input[i] == '-') {
            begin[option_num] = i + 1;
        }
        if (begin[option_num] != 0 && cmd->input[i] == ' ') {
            input_len[option_num] = i - begin[option_num];
            option_num++;
        }
    }
    // printf("option_num: %d\n", option_num);


    for (int i = 0; i < option_num; i++) {
        option_begin[i] = begin[i];
        for(int j = begin[i]; j < begin[i] + input_len[i]; j++) {
            if (cmd->input[j] == '=') {
                option_len[i] = j - option_begin[i];
                value_begin[i] = j + 1;
                value_len[i] = input_len[i] - option_len[i] - 1;
                break;
            }
        }
    }
    
    // 
    for (int i = 0; i < option_num; i++){
        // printf("value_begin[%d] : %d value_len[%d] : %d\n", i, value_begin[i], i, value_len[i]);
        char *option = (char *)malloc((option_len[i] + 1) * sizeof(char));
        strncpy(option, cmd->input + option_begin[i], option_len[i]);
        option[option_len[i]] = '\0'; 
        for (int j = 0; j < PARAMETER_NUM; j++) {
            // printf("option: %s,  cmd.parameter[j].option: %s \n", option, cmd->parameter[j].option);
            if (strcmp(option, cmd->parameter[j].option) == 0) {
                cmd->parameter[j].value = (char *)malloc((value_len[i] + 1) * sizeof(char));
                strncpy(cmd->parameter[j].value, cmd->input + value_begin[i], value_len[i]);
                cmd->parameter[j].value[value_len[i]] = '\0';                                             //
                // printf("[%s]: %s\n", cmd->parameter[j].option, cmd->parameter[j].value);
                break;
            }
        }
        free(option);
    }
}

void cmd_readline(void)
{
    // printf(">>");
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * BUF_SIZE);
    while (1) {
        int len = 0;
        uint8_t begin = 0;
        cmdline.cmd_num = 0;
        len = uart_read_bytes(UART_NUM_0, data, (BUF_SIZE - 1), 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';
            for (int i = 0; i < len; i++) {
                if (data[i] == 0x0A) {
                    // printf("%d\n", cmdline.cmd_num);

                    cmdline.cmd[cmdline.cmd_num].input = (char *)malloc(sizeof(char) * BUF_SIZE);
                    memcpy(cmdline.cmd[cmdline.cmd_num].input, data + begin, i - begin);
                    cmdline.cmd[cmdline.cmd_num].input[i - begin] = ' ';
                    cmdline.cmd[cmdline.cmd_num].input[i - begin + 1] = '\0';
                    cmdline.cmd[cmdline.cmd_num].len = i - begin + 1;
                    begin = i + 1;
                    cmdline.cmd_num++;
                }
            }
            return;
        }
    }
    free(data);
}

void cmd_exit(void)
{
    for(int j = 0; j < cmdline.cmd_num; j++){
        for (int i = 0; i < PARAMETER_NUM; i++) {
            free(cmdline.cmd[j].parameter[i].value);
        }
    }
}

esp_err_t cmd_get_value(cmd_t *cmd, const char *option, char **value)
{
    for (int i = 0; i < PARAMETER_NUM; i++) {
        if (strcmp(option, cmd->parameter[i].option) == 0) {
            *value = cmd->parameter[i].value;
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

void cmd_init(void)
{
    DebugUartInit();
    cmd_register_parameter();
}

void cmd_start(void)
{
    cmd_readline();

    for (int i = 0; i < cmdline.cmd_num; i++){
        // printf("%s\n", cmdline.cmd[i].input);
        cmd_decode(&cmdline.cmd[i]);
        cmdline.cmd[i].len = 0;
        free(cmdline.cmd[i].input);
        cmdline.cmd[i].input = NULL;
    }
}

