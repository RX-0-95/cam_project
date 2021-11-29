#include "detection_responder.h"
#include "pico/stdlib.h"
//

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score) {
  TF_LITE_REPORT_ERROR(error_reporter, "person score:%d no person score %d",
                       person_score, no_person_score);

#ifdef SEND_IMAGE_AFTER_INFERENCE
  uint8_t header[4] = {0x55, 0xBB, (uint8_t)person_score, (uint8_t)no_person_score};
  uart_write_blocking(IMAGE_UART_ID, header, 4);
#endif
}
