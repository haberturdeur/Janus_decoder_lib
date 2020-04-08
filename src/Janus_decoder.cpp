#include "Janus_decoder.h"
// #include "command_list.h"

void Decoder::init(uart_port_t i_uartPort,
    int i_rxPin,
    int i_txPin,
    int i_rtsPin,
    int i_ctsPin,
    uart_config_t* i_uartConfigPtr,
    int i_uartBufferSize,
    TickType_t i_brake)
{
    m_settings.uartPort = i_uartPort;
    m_settings.rxPin = i_rxPin;
    m_settings.txPin = i_txPin;
    m_settings.rtsPin = i_rtsPin;
    m_settings.ctsPin = i_ctsPin;
    m_settings.uartConfigPtr = i_uartConfigPtr;
    m_settings.uartBufferSize = i_uartBufferSize;
    m_settings.brake = i_brake;
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
    ESP_LOGV(TAG, "Initialized decoder.");
}

void Decoder::send(uint8_t recepientAddress, uint8_t sen_addr, uint8_t cmd, std::vector<uint8_t>& src)
{
    uint8_t length_of_data = src.size();
    std::vector<uint8_t> dataToSend = { recepientAddress, sen_addr, cmd, length_of_data };
    dataToSend.insert(dataToSend.end(), src.begin(), src.end());
    uint16_t check = crc16_le(CRC_START, dataToSend.data(), dataToSend.size());
    dataToSend.insert(dataToSend.end(),
        reinterpret_cast<uint8_t*>(&check),
        reinterpret_cast<uint8_t*>(&check) + sizeof(check));

    writeBytes(&staticStartByte, 1);
    uart_write_bytes_with_break(m_settings.uartPort, reinterpret_cast<const char*>(dataToSend.data()), dataToSend.size(), m_settings.brake);
    ESP_LOGV(TAG, "Sent data over decoder with check: %u", check);
    // ESP_LOGV(TAG, "Check of whole message is: %u",crc16_le(CRC_START, dataToSend.data(), dataToSend.size()));
}

message_t Decoder::receive()
{
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    uint8_t read_byte;
    std::vector<uint8_t> input;
    m_receivedMessage.data.clear();
    input.clear();

    static uint8_t* rec_buff = (uint8_t*)malloc(m_settings.uartBufferSize);

    do {
        readBytes(&read_byte, 1, PACKET_READ_TICS);
        ESP_LOGV(TAG, ".");
    } while (read_byte != staticStartByte);

    ESP_LOGV(TAG, "Reading message.");

    readBytes(rec_buff, LENGTH_OF_DATA_BYTE, PACKET_READ_TICS);
    input.insert(input.end(), rec_buff, rec_buff + LENGTH_OF_DATA_BYTE);

    readBytes(rec_buff, input.back() + 2, PACKET_READ_TICS);
    input.insert(input.end(), rec_buff, rec_buff + input.back() + 2);

    m_receivedMessage.recepientAddress = input[0];
    ESP_LOGV(TAG, "\tRecepient: %u", m_receivedMessage.recepientAddress);

    m_receivedMessage.senderAddress = input[1];
    ESP_LOGV(TAG, "\tSender: %u", m_receivedMessage.senderAddress);

    m_receivedMessage.cmd = input[2];
    ESP_LOGV(TAG, "\tCommand: %u", m_receivedMessage.cmd);

    m_receivedMessage.length = input[3];
    ESP_LOGV(TAG, "\tLength of data: %u", m_receivedMessage.length);

    for (int i = 4; i < (input.size() - 2); i++) {
        m_receivedMessage.data.push_back(input[i]);
        ESP_LOGV(TAG, "\tData byte%i: %u", i - 4, m_receivedMessage.data.back());
    }

    m_receivedMessage.check = *reinterpret_cast<uint16_t*>(input.data() + LENGTH_OF_DATA_BYTE + m_receivedMessage.length);
    ESP_LOGV(TAG, "\tCheck: %u\n", m_receivedMessage.check);
    // ESP_LOGV(TAG, "\tCalculated check: %u\n", crc16_le(CRC_START, input.data(), input.size() - 2));
    if (crc16_le(CRC_START, input.data(), input.size() - 2) == m_receivedMessage.check)
        ESP_LOGI(TAG, "Check does match");
    else
        ESP_LOGE(TAG, "Check doesn't match");

    return m_receivedMessage;
}

bool Decoder::act(bool i_solveAll)
{
    auto& uartPort = m_settings.uartPort;
    bool out = 0;
    uart_event_t event;
    while ((xQueueReceive(m_queue, static_cast<void*>(&event), static_cast<portTickType>(portMAX_DELAY))) && i_solveAll) {
        ESP_LOGI(TAG, "uart[%d] event:", uartPort);

        switch (event.type) {
        case UART_DATA:
            ESP_LOGI(TAG, "Received message");
            receive();
            out = 1;
            break;
        case UART_FIFO_OVF:
            ESP_LOGW(TAG, "hw fifo overflow");
            uart_flush_input(uartPort);
            xQueueReset(m_queue);
            break;
        case UART_BUFFER_FULL:
            ESP_LOGW(TAG, "ring buffer full");
            uart_flush_input(uartPort);
            xQueueReset(m_queue);
            break;
        case UART_BREAK:
            ESP_LOGI(TAG, "uart rx break");
            break;
        case UART_PARITY_ERR:
            ESP_LOGW(TAG, "uart parity error");
            break;
        case UART_FRAME_ERR:
            ESP_LOGW(TAG, "uart frame error");
            break;
        default:
            ESP_LOGI(TAG, "uart event type: %d", event.type);
            break;
        }
    }
    return out;
}