#pragma once

#include "freertos/queue.h"
#include <cstdint>
#include <driver/uart.h>
#include <vector>

enum byte_order {
    START_BYTE,
    RECEPIENT_ADDR_BYTE,
    SENDER_ADDR_BYTE,
    COMMAND_BYTE,
    LENGTH_OF_DATA_BYTE,
    FIRST_DATA_BYTE,
    CHECK_BYTE_LO,
    CHECK_BYTE_HI
};

enum message_order {
    RECEPIENT_ADDR,
    SENDER_ADDR,
    COMMAND,
    LENGHT_OF_DATA,
    DATA,
    CHECK
};

struct message_t {
    uint8_t recepientAddress;
    uint8_t senderAddress;
    uint8_t cmd;
    uint8_t length;
    uint16_t check;
    std::vector<uint8_t> data;
    bool correct { 0 };
};

struct Janus_decoder_settings_t {
    uart_port_t uartPort = UART_NUM_2;
    int rxPin = DEFAULT_PIN_RXD;
    int txPin = DEFAULT_PIN_TXD;
    int rtsPin = DEFAULT_PIN_RTS;
    int ctsPin = DEFAULT_PIN_CTS;
    uart_config_t* uartConfigPtr = &defaultUartConfig;
    int uartBufferSize = BUF_SIZE;
    TickType_t brake = 16; // 2 * 8bits = 2 chars
};