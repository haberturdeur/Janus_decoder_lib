#include "Janus_decoder.h"
// #include "command_list.h"

void Decoder::init(uart_port_t i_uartPort,
    int i_rxPin,
    int i_txPin,
    int i_rtsPin,
    int i_ctsPin,
    uart_config_t* i_uartConfigPtr,
    int i_uartBufferSize)
{
    m_settings.uartPort = i_uartPort;
    m_settings.rxPin = i_rxPin;
    m_settings.txPin = i_txPin;
    m_settings.rtsPin = i_rtsPin;
    m_settings.ctsPin = i_ctsPin;
    m_settings.uartConfigPtr = i_uartConfigPtr;
    m_settings.uartBufferSize = i_uartBufferSize;
    init(m_settings);
}

void Decoder::init(Janus_decoder_settings_t settings)
{
    m_uartPort = settings.uartPort;
    m_bufferSize = settings.uartBufferSize;
    if (settings.rxPin == -1)
        settings.rxPin = DEFAULT_PIN_RXD;
    if (settings.txPin == -1)
        settings.txPin = DEFAULT_PIN_TXD;
    if (settings.rtsPin == -1)
        settings.rtsPin = DEFAULT_PIN_RTS;
    if (settings.ctsPin == -1)
        settings.ctsPin = DEFAULT_PIN_CTS;
    uart_param_config(m_uartPort, settings.uartConfigPtr);
    uart_set_pin(m_uartPort, settings.txPin, settings.rxPin, settings.rtsPin, settings.ctsPin);
    uart_driver_install(m_uartPort, m_bufferSize * 2, 0, 0, NULL, 0);
    uart_set_mode(m_uartPort, UART_MODE_RS485_HALF_DUPLEX);
}

void Decoder::send(uint8_t recepientAddress, uint8_t sen_addr, uint8_t cmd, std::vector<uint8_t>& src)
{
    uint8_t length_of_data = src.size();
    message_t out;
    out.cmd = cmd;
    out.data = src;
    out.length = length_of_data;
    out.recepientAddress = recepientAddress;
    out.senderAddress = sen_addr;

    out.check = calculateChecksum(out);
    uart_write_bytes(m_uartPort, &saticStartByte, 1);
    uart_write_bytes(m_uartPort, (const char*)&recepientAddress, 1);
    uart_write_bytes(m_uartPort, (const char*)&sen_addr, 1);
    uart_write_bytes(m_uartPort, (const char*)&cmd, 1);
    uart_write_bytes(m_uartPort, (const char*)&length_of_data, 1);
    uart_write_bytes(m_uartPort, (const char*)&out.check, 1);
    char out_data = 0;
    for (int i = 0; i < length_of_data; i++) {
        out_data = src[i];
        uart_write_bytes(m_uartPort, (const char*)(&out_data), 1);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
}

message_t Decoder::receive()
{
    uint8_t read_byte;
    m_receivedMessage.data.clear();
#if debug_decoder
    printf("Receiving message");
#endif
    static uint8_t* rec_buff = (uint8_t*)malloc(m_bufferSize);
    do {
        uart_read_bytes(m_uartPort, &read_byte, 1, PACKET_READ_TICS);
#if debug_decoder
        printf(".");
#endif
    } while (read_byte != saticStartByte);
#if debug_decoder
    printf("\n Parsing message.\n");
#endif
    uart_read_bytes(m_uartPort, rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.recepientAddress = rec_buff[0];
#if debug_decoder
    printf("Recepient: %u\n", m_receivedMessage.recepientAddress);
#endif
    uart_read_bytes(m_uartPort, rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.senderAddress = rec_buff[0];
#if debug_decoder
    printf("Sender: %u\n", m_receivedMessage.senderAddress);
#endif
    uart_read_bytes(m_uartPort, rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.cmd = rec_buff[0];
#if debug_decoder
    printf("Command: %u\n", m_receivedMessage.cmd);
    printf((const char*)&m_receivedMessage.cmd);
#endif
    uart_read_bytes(m_uartPort, rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.length = rec_buff[0];
#if debug_decoder
    printf("Length: %u\n", m_receivedMessage.length);
#endif
    uart_read_bytes(m_uartPort, rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.check = rec_buff[0];
#if debug_decoder
    printf("Check: %u\n", m_receivedMessage.check);
    printf("Data: ");
#endif
    for (int i = 0; i < m_receivedMessage.length; i++) {
        uart_read_bytes(m_uartPort, rec_buff, 1, PACKET_READ_TICS);
        m_receivedMessage.data.push_back(rec_buff[0]);
#if debug_decoder
        printf("%u", m_receivedMessage.data[i]);
#endif
    }
    // received_message = parse_message(rec_buff);
    uart_flush(m_uartPort);

    if (calculateChecksum(m_receivedMessage) == m_receivedMessage.check)
        m_receivedMessage.correct = 1;
    else
        m_receivedMessage.correct = 0;
#if debug_decoder
    printf("Check_calc: %u\n", calculateChecksum(m_receivedMessage));
    printf("Check_sum check: %u\n", m_receivedMessage.correct);
#endif

    return m_receivedMessage;
}

uint8_t Decoder::calculateChecksum(message_t& in)
{
    uint32_t check_calc = 0;

    check_calc += in.recepientAddress;
    check_calc += in.senderAddress;
    check_calc += in.cmd;
    check_calc += in.length;
    for (auto& i : in.data)
        check_calc += in.data[i];
    uint8_t out = check_calc % 256;
    return out;
}

message_t Decoder::parseMessage(uint8_t* msg)
{
    message_t output;
#if debug_decoder
    printf("Parsing message.\n");
#endif
    output.recepientAddress = msg[(RECEPIENT_ADDR * sizeof(uint8_t))];
#if debug_decoder
    printf("Recepient: %u\n", output.recepientAddress);
#endif
    output.senderAddress = msg[(SENDER_ADDR * sizeof(uint8_t))];
#if debug_decoder
    printf("Sender: %u\n", output.senderAddress);
#endif
    output.cmd = msg[(COMMAND * sizeof(uint8_t))];
#if debug_decoder
    printf("Command: %u\n", output.cmd);
    printf((const char*)&output.cmd);
    printf("Data: ");
#endif

    output.check = msg[(CHECK_BYTE * sizeof(uint8_t))];
#if debug_decoder
    printf("Check: %u\n", output.check);
#endif
    output.data.clear();
    for (int i = 0; i < output.length; i++) {
        output.data.push_back(msg[(FIRST_DATA_BYTE * sizeof(uint8_t)) + (i * sizeof(uint8_t))]);
#if debug_decoder
        printf("%u\n", output.data[i]);
#endif
    }

    int check_calc = 0;

    check_calc += output.recepientAddress;
    check_calc += output.senderAddress;
    check_calc += output.cmd;
    for (auto& i : output.data)
        check_calc += output.data[i];
    check_calc = check_calc % 256;
    if (check_calc == output.check)
        output.correct = 1;
    else
        output.correct = 0;
#if debug_decoder
    printf("Check_calc: %u\n", check_calc);
    printf("Check_sum check: %u\n", output.correct);
#endif
    return output;
}