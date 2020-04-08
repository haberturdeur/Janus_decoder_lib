#include "Janus_decoder.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <vector>

Decoder decoder;
std::vector<uint8_t> a = { 5, 6, 7 };
extern "C" void app_main()
{
    decoder.init(UART_NUM_2);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    while (1) {
        decoder.send(2, 3, 4, a);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}