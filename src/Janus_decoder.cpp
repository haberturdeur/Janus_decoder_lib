
#include "Janus_decoder.h"
// #include "command_list.h"


void Decoder::init(uart_port_t u_port, 
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

void Decoder::init(Janus_decoder_settings_t settings){
    uart_port = settings.u_port;
    buffer_size = settings.buff_size;
    if(settings.rx_p == -1) settings.rx_p = DEFAULT_PIN_RXD;
    if(settings.tx_p == -1) settings.tx_p = DEFAULT_PIN_TXD;
    if(settings.rts_p == -1) settings.rts_p = DEFAULT_PIN_RTS;
    if(settings.cts_p == -1) settings.cts_p = DEFAULT_PIN_CTS;
    uart_param_config(uart_port, settings.u_config);
    uart_set_pin(uart_port, settings.tx_p, settings.rx_p, settings.rts_p, settings.cts_p);
    uart_driver_install(uart_port, buffer_size * 2, 0, 0, NULL, 0);
    uart_set_mode(uart_port, UART_MODE_RS485_HALF_DUPLEX);
}

void Decoder::send(uint8_t rec_addr,uint8_t sen_addr, uint8_t cmd, std::vector<uint8_t>& src){
    uint8_t length_of_data = src.size();
    message_t out;
    out.cmd = cmd;
    out.data = src;
    out.length = length_of_data;
    out.rec_addr = rec_addr;
    out.send_addr = sen_addr;

    out.check = calculateChecksum(out);
    uart_write_bytes(uart_port, &SOH, 1);
    uart_write_bytes(uart_port, (const char*)&rec_addr, 1);
    uart_write_bytes(uart_port, (const char*)&sen_addr, 1);
    uart_write_bytes(uart_port, (const char*)&cmd, 1);
    uart_write_bytes(uart_port, (const char*)&length_of_data, 1);
    uart_write_bytes(uart_port, (const char*)&out.check, 1);
    char out_data = 0;
    for(int i = 0; i<length_of_data; i++){
        out_data = src[i];
        uart_write_bytes(uart_port, (const char*)(&out_data), 1);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
}

message_t Decoder::receive(){
    uint8_t read_byte;
    received_message.data.clear();
        #if debug_decoder
            printf("Receiving message");
            #endif
    static uint8_t* rec_buff = (uint8_t*) malloc(buffer_size);
    do{
        uart_read_bytes(uart_port, &read_byte, 1, PACKET_READ_TICS);
        #if debug_decoder
            printf(".");
            #endif
    }while (read_byte != SOH);
        #if debug_decoder
            printf("\n Parsing message.\n");
            #endif
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.rec_addr = rec_buff[0];
        #if debug_decoder
            printf("Recepient: %u\n", received_message.rec_addr);
            #endif
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.send_addr = rec_buff[0];
        #if debug_decoder
            printf("Sender: %u\n", received_message.send_addr);
            #endif
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.cmd = rec_buff[0];
        #if debug_decoder
            printf("Command: %u\n", received_message.cmd);
            printf((const char*)&received_message.cmd);
            #endif
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.length = rec_buff[0];
        #if debug_decoder
            printf("Length: %u\n", received_message.length);
            #endif
    uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
    received_message.check = rec_buff[0];
    #if debug_decoder
            printf("Check: %u\n", received_message.check);
            printf("Data: ");
            #endif
    for(int i = 0; i<received_message.length; i++){
        uart_read_bytes(uart_port, rec_buff, 1, PACKET_READ_TICS);
        received_message.data.push_back(rec_buff[0]);
        #if debug_decoder
            printf("%u", received_message.data[i]);
            #endif
    } 
    // received_message = parse_message(rec_buff);
    uart_flush(uart_port);

    
    if(calculateChecksum(received_message)==received_message.check) 
        received_message.correct = 1; 
        else 
        received_message.correct = 0;
        #if debug_decoder
            printf("Check_calc: %u\n", calculateChecksum(received_message));
            printf("Check_sum check: %u\n", received_message.correct);
        #endif

    return received_message;
}

uint8_t Decoder::calculateChecksum(message_t &in){
    uint32_t check_calc = 0;

    check_calc += in.rec_addr;
    check_calc += in.send_addr;
    check_calc += in.cmd;
    check_calc += in.length;
    for( auto &i : in.data)
        check_calc += in.data[i];
    uint8_t out = check_calc % 256;
    return out;
}



message_t Decoder::parse_message(uint8_t* msg){
    message_t output;
        #if debug_decoder
            printf("Parsing message.\n");
            #endif
    output.rec_addr = msg[(RECEPIENT_ADDR*sizeof(uint8_t))];
        #if debug_decoder
            printf("Recepient: %u\n", output.rec_addr);
            #endif
    output.send_addr = msg[(SENDER_ADDR*sizeof(uint8_t))];
        #if debug_decoder
            printf("Sender: %u\n", output.send_addr);
            #endif
    output.cmd = msg[(COMMAND*sizeof(uint8_t))];
        #if debug_decoder
            printf("Command: %u\n", output.cmd);
            printf((const char*)&output.cmd);
            printf("Data: ");
            #endif

    
    
    output.check = msg[(CHECK_BYTE*sizeof(uint8_t))];
        #if debug_decoder
            printf("Check: %u\n", output.check);
            #endif
    output.data.clear();
    for(int i = 0; i< output.length; i++){
        output.data.push_back(msg[(FIRST_DATA_BYTE*sizeof(uint8_t))+(i*sizeof(uint8_t))]);
            #if debug_decoder
            printf("%u\n", output.data[i]);
            #endif
    }


    int check_calc = 0;

    check_calc += output.rec_addr;
    check_calc += output.send_addr;
    check_calc += output.cmd;
    for( auto &i : output.data)
        check_calc += output.data[i];
    check_calc = check_calc % 256;
    if(check_calc==output.check) 
        output.correct = 1; 
        else 
        output.correct = 0;
        #if debug_decoder
            printf("Check_calc: %u\n", check_calc);
            printf("Check_sum check: %u\n", output.correct);
        #endif
    return output;
}