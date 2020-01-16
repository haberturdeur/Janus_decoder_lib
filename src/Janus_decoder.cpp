
#include "Janus_decoder.h"
// #include "command_list.h"


Decoder::Decoder(uart_port_t u_port, 
            int rx_p, 
            int tx_p, 
            int rts_p, 
            int cts_p, 
            uart_config_t* u_config,
            int buff_size){
    uart_port = u_port;
    buffer_size = buff_size;
    if(rx_p == -1) rx_p = DEFAULT_PIN_RXD;
    if(tx_p == -1) tx_p = DEFAULT_PIN_TXD;
    if(rts_p == -1) rts_p = DEFAULT_PIN_RTS;
    if(cts_p == -1) cts_p = DEFAULT_PIN_CTS;
    uart_param_config(uart_port, u_config);
    uart_set_pin(uart_port, tx_p, rx_p, rts_p, cts_p);
    uart_driver_install(uart_port, buffer_size * 2, 0, 0, NULL, 0);
    uart_set_mode(uart_port, UART_MODE_RS485_HALF_DUPLEX);
}

void Decoder::send(uint8_t rec_addr,uint8_t sen_addr, uint8_t cmd, std::vector<uint8_t>* src){
    uint8_t length_of_data = src->size();
    uint8_t control = rec_addr + sen_addr + cmd;
    for(int i = 0; i<length_of_data; i++){
        control += (*src)[i];
    }
    control = control % 256;
        #if debug
            printf("Check: %u\n", control);
        #endif
    uart_write_bytes(uart_port, &SOH, 1);
    uart_write_bytes(uart_port, (const char*)&rec_addr, 1);
    uart_write_bytes(uart_port, (const char*)&sen_addr, 1);
    uart_write_bytes(uart_port, (const char*)&cmd, 1);
    uart_write_bytes(uart_port, (const char*)&length_of_data, 1);
    uart_write_bytes(uart_port, (const char*)&control, 1);
    char out_data = 0;
    for(int i = 0; i<length_of_data; i++){
        out_data = (*src)[i];
        uart_write_bytes(uart_port, (const char*)(&out_data), 1);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
}

message_t Decoder::receive(){
    uint8_t read_byte;
    received_message.data.clear();
    static uint8_t* rec_buff = (uint8_t*) malloc(buffer_size);
    do{
        uart_read_bytes(uart_port, &read_byte, 1, PACKET_READ_TICS);
    }while (read_byte != SOH);
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.rec_addr = rec_buff[0];
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.send_addr = rec_buff[0];
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.cmd = rec_buff[0];
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.length = rec_buff[0];
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.check = rec_buff[0];
    for(int i = 0; i<received_message.length; i++){
        uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
        received_message.data.push_back(rec_buff[0]);
    } 
    // received_message = parse_message(rec_buff);
    uart_flush(uart_port);
    return received_message;
}



message_t Decoder::parse_message(uint8_t* msg){
    message_t output;
        #if debug
            printf("Parsing message.\n");
            #endif
    output.rec_addr = msg[(RECEPIENT_ADDR*sizeof(uint8_t))];
        #if debug
            printf("Recepient: %u\n", output.rec_addr);
            #endif
    output.send_addr = msg[(SENDER_ADDR*sizeof(uint8_t))];
        #if debug
            printf("Sender: %u\n", output.send_addr);
            #endif
    output.cmd = msg[(COMMAND*sizeof(uint8_t))];
        #if debug
            printf("Command: %u\n", output.cmd);
            printf((const char*)&output.cmd);
            printf("Data: ");
            #endif

    // output.data[0] = msg[(FIRST_DATA*sizeof(uint8_t))];
    //     #if debug
    //         printf("Data: %u", output.data[0]);
    //         #endif
    // output.data[1] = msg[(SECOND_DATA*sizeof(uint8_t))];
    //     #if debug
    //         printf("%u", output.data[1]);
    //         #endif
    // output.data[2] = msg[(THIRD_DATA*sizeof(uint8_t))];
    //     #if debug
    //         printf("%u", output.data[2]);
    //         #endif
    // output.data[3] = msg[(FOURTH_DATA*sizeof(uint8_t))];
    //     #if debug
    //         printf("%u", output.data[3]);
    //         #endif
    // output.data[4] = msg[(FIFTH_DATA*sizeof(uint8_t))];
    //     #if debug
    //         printf("%u\n", output.data[4]);
    //         #endif

    output.check = msg[(CHECK_BYTE*sizeof(uint8_t))];
        #if debug
            printf("Check: %u\n", output.check);
            #endif
    
    int check_calc = 0;

    check_calc += output.rec_addr;
    check_calc += output.send_addr;
    check_calc += output.cmd;
    for(int i = 0; i< 5; i++) 
        check_calc += output.data[i];
    check_calc = check_calc % 256;
    if(check_calc==output.check) 
        output.correct = 1; 
        else 
        output.correct = 0;
        #if debug
            printf("Check_calc: %u\n", check_calc);
            printf("Check_sum check: %u\n", output.correct);
        #endif
    return output;
}