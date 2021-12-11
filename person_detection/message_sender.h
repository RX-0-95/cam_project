#ifndef _MESSAGE_SENDER_H_
#define _MESSAGE_SENDER_H_

#include "detect_config.h"
#include "pico/stdlib.h"


struct priority_level
{
    /* data */
    uint8_t p1;
    uint8_t p2;
    uint8_t p3;
    uint32_t p_len; 
};


struct messages_sender{
    uint8_t header[2];
    uint32_t header_len;
    uint8_t* tailer;
    uint32_t tailer_len; 
    void(*send_message)(uint8_t* msg, uint32_t msg_size);
};

#endif