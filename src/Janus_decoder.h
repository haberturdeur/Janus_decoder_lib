#pragma once

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "configuration.h"

#include "Janus_decoder_types.h"
#include "driver/uart.h"
#include "esp32/rom/crc.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include <string.h>
#include <vector>

static const Janus_decoder_settings_t JanusDecoderDefaultSettings;

class Decoder {
private:
    uint8_t m_myAddress = 0x00;
    message_t m_receivedMessage;
    Janus_decoder_settings_t m_settings;
    int writeBytes(const char* i_src, size_t i_size) { return uart_write_bytes(m_settings.uartPort, i_src, i_size); }
    int readBytes(uint8_t* i_buffer, uint32_t i_length, TickType_t i_ticksToWait) { return uart_read_bytes(m_settings.uartPort, i_buffer, i_length, i_ticksToWait); }
    QueueHandle_t m_queue;

public:
    void init(uart_port_t i_uartPort,
        int i_rxPin = DEFAULT_PIN_RXD,
        int i_txPin = DEFAULT_PIN_TXD,
        int i_rtsPin = DEFAULT_PIN_RTS,
        int i_ctsPin = DEFAULT_PIN_CTS,
        uart_config_t* i_uartConfigPtr = &defaultUartConfig,
        int i_uartBufferSize = BUF_SIZE,
        TickType_t i_brake = 16);
    void init(Janus_decoder_settings_t i_settings = JanusDecoderDefaultSettings);
    void send(
        uint8_t i_recepientAddress,
        uint8_t i_senderAddress,
        uint8_t i_cmd,
        std::vector<uint8_t>& i_data);
    message_t receive();
    Janus_decoder_settings_t setting() { return m_settings; }
    uint8_t myAddress() { return m_myAddress; }
    message_t receivedMessage() { return m_receivedMessage; }
    void act()
    {
        auto& bufferSize = m_settings.uartBufferSize;
        auto& uartPort = m_settings.uartPort;
        uart_event_t event;
        uint8_t* dtmp = (uint8_t*)malloc(bufferSize);
        if (xQueueReceive(m_queue, static_cast<void*>(&event), static_cast<portTickType>(portMAX_DELAY))) {
            // bzero(dtmp, bufferSize);
            ESP_LOGI(TAG, "uart[%d] event:", uartPort);
            switch (event.type) {
            case UART_DATA:
                // ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                // uart_read_bytes(uartPort, dtmp, event.size, portMAX_DELAY);
                // ESP_LOGI(TAG, "[DATA EVT]:");
                // for (int i = 0; i < event.size; i++) {
                //     ESP_LOGI(TAG, "%d", dtmp[i]);
                // }
                // uart_write_bytes(uartPort, (const char*) dtmp, event.size);
                receive();
                break;
            // case UART_FIFO_OVF:
            //     ESP_LOGI(TAG, "hw fifo overflow");
            //     uart_flush_input(uartPort);
            //     xQueueReset(m_queue);
            //     break;
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                uart_flush_input(uartPort);
                xQueueReset(m_queue);
                break;
            // case UART_BREAK:
            //     ESP_LOGI(TAG, "uart rx break");
            //     break;
            // case UART_PARITY_ERR:
            //     ESP_LOGI(TAG, "uart parity error");
            //     break;
            // case UART_FRAME_ERR:
            //     ESP_LOGI(TAG, "uart frame error");
            //     break;
            //Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
        free(dtmp);
        dtmp = NULL;
    }
};
