#pragma once

#include <vector>
#include "driver/uart.h"

// Note: UART2 default pins IO16, IO17 do not work on ESP32-WROVER module 
// because these pins connected to PSRAM

#define wrover false
#define debug_decoder true

#if wrover
    #define DEFAULT_PIN_TXD   (23)//23 
    #define DEFAULT_PIN_RXD   (22)//22
#else
    #define DEFAULT_PIN_TXD   (17)//23 
    #define DEFAULT_PIN_RXD   (16)//22
#endif

// RTS for RS485 Half-Duplex Mode manages DE/~RE
#define DEFAULT_PIN_RTS   (18)

// CTS is not used in RS485 Half-Duplex Mode
#define DEFAULT_PIN_CTS  UART_PIN_NO_CHANGE

#define BUF_SIZE        127
#define BAUD_RATE       (115200)

// Read packet timeout
#define PACKET_READ_TICS        (100 / portTICK_RATE_MS)
#define ECHO_TASK_STACK_SIZE    (2048)
#define ECHO_TASK_PRIO          (10)

static uart_config_t default_uart_config = {
    .baud_rate = BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
};

enum byte_order{
    START_BYTE,
    RECEPIENT_ADDR_BYTE,
    SENDER_ADDR_BYTE,
    COMMAND_BYTE,
    LENGTH_OF_DATA_BYTE,
    CHECK_BYTE,
    FIRST_DATA_BYTE
};

enum message_order{
    RECEPIENT_ADDR,
    SENDER_ADDR,
    COMMAND,
    LENGHT_OF_DATA,
    CHECK,
    FIRST_DATA
};

const char SOH = 0x80;

struct message_t
{
    uint8_t rec_addr;
    uint8_t send_addr;
    uint8_t cmd;
    uint8_t length;
    uint8_t check;
    std::vector<uint8_t> data;
    bool correct{0};
};

struct Janus_decoder_settings_t
{
    uart_port_t u_port = UART_NUM_2; 
    int rx_p = DEFAULT_PIN_RXD; 
    int tx_p = DEFAULT_PIN_TXD; 
    int rts_p = DEFAULT_PIN_RTS; 
    int cts_p = DEFAULT_PIN_CTS;
    uart_config_t* u_config = &default_uart_config;
    int buff_size = BUF_SIZE;
};



class Decoder
{
private:
    uart_port_t uart_port;
    int buffer_size;
    static Janus_decoder_settings_t Janus_decoder_default_settings;
protected:
    uint8_t calculateChecksum(message_t &in);
public:    

    void init(uart_port_t u_port, 
            int rx_p = DEFAULT_PIN_RXD, 
            int tx_p = DEFAULT_PIN_TXD, 
            int rts_p = DEFAULT_PIN_RTS, 
            int cts_p = DEFAULT_PIN_CTS,
            uart_config_t* u_config = &default_uart_config,
            int buff_size = BUF_SIZE);
    void init(Janus_decoder_settings_t settings = Janus_decoder_default_settings);
    void send(
            uint8_t rec_addr,
            uint8_t sen_addr,
            uint8_t cmd,
            std::vector<uint8_t>& src);
    message_t receive();
    uint8_t my_addr = 0x00;
    message_t received_message;
    message_t parse_message(uint8_t* msg);
    
    Decoder& getDecoder(){
        return *this;
    }
    
};