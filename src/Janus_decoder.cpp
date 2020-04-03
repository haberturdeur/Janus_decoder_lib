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
    m_settings = settings;
    if (m_settings.rxPin == -1)
        m_settings.rxPin = DEFAULT_PIN_RXD;
    if (m_settings.txPin == -1)
        m_settings.txPin = DEFAULT_PIN_TXD;
    if (m_settings.rtsPin == -1)
        m_settings.rtsPin = DEFAULT_PIN_RTS;
    if (m_settings.ctsPin == -1)
        m_settings.ctsPin = DEFAULT_PIN_CTS;
    uart_param_config(m_settings.uartPort, m_settings.uartConfigPtr);
    uart_set_pin(m_settings.uartPort, m_settings.txPin, m_settings.rxPin, m_settings.rtsPin, m_settings.ctsPin);
    uart_driver_install(m_settings.uartPort, m_settings.uartBufferSize * 2, m_settings.uartBufferSize * 2, 20, &m_queue, 0);
    uart_set_mode(m_settings.uartPort, UART_MODE_RS485_HALF_DUPLEX);
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
    writeBytes(&saticStartByte, 1);
    writeBytes((const char*)&recepientAddress, 1);
    writeBytes((const char*)&sen_addr, 1);
    writeBytes((const char*)&cmd, 1);
    writeBytes((const char*)&length_of_data, 1);
    writeBytes((const char*)&out.check, 1);
    char out_data = 0;
    for (int i = 0; i < length_of_data; i++) {
        out_data = src[i];
        writeBytes((const char*)(&out_data), 1);
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
    static uint8_t* rec_buff = (uint8_t*)malloc(m_settings.uartBufferSize);
    do {
        readBytes( &read_byte, 1, PACKET_READ_TICS);
#if debug_decoder
        printf(".");
#endif
    } while (read_byte != saticStartByte);
#if debug_decoder
    printf("\n Parsing message.\n");
#endif
    readBytes( rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.recepientAddress = rec_buff[0];
#if debug_decoder
    printf("Recepient: %u\n", m_receivedMessage.recepientAddress);
#endif
    readBytes( rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.senderAddress = rec_buff[0];
#if debug_decoder
    printf("Sender: %u\n", m_receivedMessage.senderAddress);
#endif
    readBytes( rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.cmd = rec_buff[0];
#if debug_decoder
    printf("Command: %u\n", m_receivedMessage.cmd);
    printf((const char*)&m_receivedMessage.cmd);
#endif
    readBytes( rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.length = rec_buff[0];
#if debug_decoder
    printf("Length: %u\n", m_receivedMessage.length);
#endif
    readBytes( rec_buff, 1, PACKET_READ_TICS);
    m_receivedMessage.check = rec_buff[0];
#if debug_decoder
    printf("Check: %u\n", m_receivedMessage.check);
    printf("Data: ");
#endif
    for (int i = 0; i < m_receivedMessage.length; i++) {
        readBytes( rec_buff, 1, PACKET_READ_TICS);
        m_receivedMessage.data.push_back(rec_buff[0]);
#if debug_decoder
        printf("%u", m_receivedMessage.data[i]);
#endif
    }
    // received_message = parse_message(rec_buff);
    uart_flush(m_settings.uartPort);

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