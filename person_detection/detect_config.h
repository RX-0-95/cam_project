#ifndef _DETECT_CONFIG_H_
#define _DETECT_CONFIG_H_

#include "hardware/uart.h" 
#include "pico/stdlib.h"
#include "arducam.h"


//allow image trasnfer after capture
#define SEND_IMAGE_AFTER_CAPTURE

//allow image transfer after inference
//#define SEND_IMAGE_AFTER_INFERENCE

// define uart settings 
#define IMAGE_UART_ID uart1
#define IMAGE_TX_PIN 20
#define IMAGE_BAUD_RATE 921600
#define IMAGE_DATA_BITS 8
#define IMAGE_STOP_BITS 1
#define IMAGE_PARITY    UART_PARITY_NONE
#define IMAGE_UART_TX_PIN 20
#define IMAGE_UART_RX_PIN 21

// motion detect conifg
#define BITMAP_THRESHOLD 32
#define DETECT_PERCENT_THRESHOLD 0.05

//JPEG capeture trasnsfer
#define CAPTURE_FRAME_RATE 4

#endif