#pragma once

#include "config.h"

#include "driver/uart.h"
#include <vector>

static uart_config_t defaultUartConfig = {
    .baud_rate = BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
};

enum byte_order {
    START_BYTE,
    RECEPIENT_ADDR_BYTE,
    SENDER_ADDR_BYTE,
    COMMAND_BYTE,
    LENGTH_OF_DATA_BYTE,
    CHECK_BYTE,
    FIRST_DATA_BYTE
};

enum message_order {
    RECEPIENT_ADDR,
    SENDER_ADDR,
    COMMAND,
    LENGHT_OF_DATA,
    CHECK,
    FIRST_DATA
};

struct message_t {
    uint8_t recepientAddress;
    uint8_t senderAddress;
    uint8_t cmd;
    uint8_t length;
    uint8_t check;
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
};

static const Janus_decoder_settings_t JanusDecoderDefaultSettings;

class Decoder {
private:
    uart_port_t m_uartPort;
    int m_bufferSize;
    uint8_t m_myAddress = 0x00;
    message_t m_receivedMessage;
    Janus_decoder_settings_t m_settings;
    uint8_t calculateChecksum(message_t& in);
    // sendByte

public:
    void init(uart_port_t i_uartPort,
        int i_rxPin = DEFAULT_PIN_RXD,
        int i_txPin = DEFAULT_PIN_TXD,
        int i_rtsPin = DEFAULT_PIN_RTS,
        int i_ctsPin = DEFAULT_PIN_CTS,
        uart_config_t* i_uartConfigPtr = &defaultUartConfig,
        int i_uartBufferSize = BUF_SIZE);
    void init(Janus_decoder_settings_t i_settings = JanusDecoderDefaultSettings);
    void send(
        uint8_t i_recepientAddress,
        uint8_t i_senderAddress,
        uint8_t i_cmd,
        std::vector<uint8_t>& i_data);
    message_t receive();
    message_t parseMessage(uint8_t* msg);
    Janus_decoder_settings_t setting() { return m_settings; }
    uint8_t myAddress() { return m_myAddress; }
    message_t receivedMessage() { return m_receivedMessage; }
};