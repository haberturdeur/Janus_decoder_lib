#include <Arduino.h>
#include "Janus_decoder.h"
#include <iostream>

Decoder decoder;

Janus_decoder_settings_t settings;
void setup() {
  settings.uartPort = UART_NUM_2;
  decoder.init(settings);
}

std::vector<uint8_t> data= {0x14, 0x15, 0x16, 0x17, 0x18, 0x20};
using namespace std;
void loop() {
  message_t out;
  // put your main code here, to run repeatedly:
  decoder.send(0x11, 0x12, 0x13, data);
  // out = decoder.receive();


  // delay(100);
}