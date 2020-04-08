#include "Janus_decoder.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

Decoder decoder;
std::vector<uint8_t> a = { 5, 6, 7 };
extern "C" void app_main()
{
    decoder.init(UART_NUM_2);
    // static const char* TEST_TAG = "Receiver_test: ";
    // esp_log_level_set(TEST_TAG, ESP_LOG_INFO);
    while (1) {
        decoder.act();
        // ESP_LOGD(TEST_TAG, "Received message:\n\tRecepient: %hhu\n\tSender: %hhu\n\tCmd: %hhu\n\tData Length: %hhu\n\tCheck: %hu\n\tData: %s", msg.recepientAddress, msg.senderAddress, msg.cmd, msg.length, msg.check, data.c_str);
    }
}