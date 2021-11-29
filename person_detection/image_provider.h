#ifndef _PERSON_DETECTION_IMAGE_PROVIDER
#define _PERSON_DETECTION_IMAGE_PROVIDER

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "detect_config.h"

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data);

#endif 