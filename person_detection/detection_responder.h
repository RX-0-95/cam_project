#ifndef _DETECTION_RESPONDER_H_
#define _DETECTION_RESPONDER_H_

#include "detect_config.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"


void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score);

#endif//_DETECTION_RESPONDER_H_