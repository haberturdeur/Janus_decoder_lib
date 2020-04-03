#pragma once

#include "driver/uart.h"

// Note: UART2 default pins IO16, IO17 do not work on ESP32-WROVER module
// because these pins connected to PSRAM

#define wrover false
#define debug_decoder true

#if wrover
#define DEFAULT_PIN_TXD (23) //23
#define DEFAULT_PIN_RXD (22) //22
#else
#define DEFAULT_PIN_TXD (17) //23
#define DEFAULT_PIN_RXD (16) //22
#endif

// RTS for RS485 Half-Duplex Mode manages DE/~RE
#define DEFAULT_PIN_RTS (18)

// CTS is not used in RS485 Half-Duplex Mode
#define DEFAULT_PIN_CTS UART_PIN_NO_CHANGE

#define BUF_SIZE 127
#define BAUD_RATE (115200)

// Read packet timeout
#define PACKET_READ_TICS (100 / portTICK_RATE_MS)
#define ECHO_TASK_STACK_SIZE (2048)
#define ECHO_TASK_PRIO (10)

static const char saticStartByte = 0x80;

static const char *TAG = "Janus_decoder";

static uart_config_t defaultUartConfig = {
    .baud_rate = BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
};