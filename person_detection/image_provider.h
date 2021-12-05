#ifndef _PERSON_DETECTION_IMAGE_PROVIDER
#define _PERSON_DETECTION_IMAGE_PROVIDER

#include "pico/stdlib.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "detect_config.h"

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data);

TfLiteStatus GetJPEGImageTransfer(tflite::ErrorReporter*error_reporter);

TfLiteStatus GetYUVImage(tflite::ErrorReporter*error_reporter,uint8_t* imageDat);

static inline void uart_transfer(uint8_t* usart_buffer,uint32_t buffer_length){
    return uart_write_blocking(IMAGE_UART_ID,usart_buffer,buffer_length);
}

#endif 

