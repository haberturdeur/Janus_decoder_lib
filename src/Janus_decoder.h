#pragma once

#include "config.h"

#include "driver/uart.h"
#include <vector>
#include "Janus_decoder_types.h"

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