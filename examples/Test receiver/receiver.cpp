#include <Arduino.h>
#include "Janus_decoder.h"

Decoder decoder(UART_NUM_2, -1, -1, -1, -1);
void setup() {
  
}
void loop() {
  // put your main code here, to run repeatedly:
  message_t r_message;

  r_message = decoder.receive();
  delay(100);
}