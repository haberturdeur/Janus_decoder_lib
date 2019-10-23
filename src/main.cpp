#include <Arduino.h>
#include "protocol.h"

Decoder decoder(UART_NUM_2, -1, -1, -1, -1);
void setup() {
  
}
uint8_t data[] = {0x14, 0x15, 0x16};
void loop() {
  // put your main code here, to run repeatedly:
  // decoder.send(0x11, 0x12, 0x13,data,sizeof(data)/8);
  message_t r_message;
  
  r_message = decoder.receive();
  delay(100);
}