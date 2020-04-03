#pragma once

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

const char saticStartByte = 0x80;