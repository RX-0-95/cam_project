#ifndef _TASK_PRETASK_H_
#define _TASK_PRETASK_H_

#include "detect_config.h"
#include "FreeRtos.h"
#include "task.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "tensorflow/lite/micro/micro_time.h"

#include "detection_responder.h"
#include "image_provider.h"
#include "model_settings.h"
#include "person_detection_model_data.h"
#include <climits>

/*
#ifdef __cplusplus
extern "C" {
#endif
*/
#ifndef TF_LITE_MICRO_EXECUTION_TIME_BEGIN 
#define TF_LITE_MICRO_EXECUTION_TIME_BEGIN      \
  int32_t start_ticks;                          \
  int32_t duration_ticks;                       \
  int32_t duration_ms;
#endif //TF_LITE_MICRO_EXECUTION_TIME_BEGIN 

#ifndef TF_LITE_MICRO_EXECUTION_TIME
#define TF_LITE_MICRO_EXECUTION_TIME(reporter, func)                    \
  if (tflite::ticks_per_second() == 0) {                                \
    TF_LITE_REPORT_ERROR(reporter,                                      \
                         "no timer implementation found");              \
  }                                                                     \
  start_ticks = tflite::GetCurrentTimeTicks();                          \
  func;                                                                 \
  duration_ticks = tflite::GetCurrentTimeTicks() - start_ticks;         \
  if (duration_ticks > INT_MAX / 1000) {                                \
    duration_ms = duration_ticks / (tflite::ticks_per_second() / 1000); \
  } else {                                                              \
    duration_ms = (duration_ticks * 1000) / tflite::ticks_per_second(); \
  }                                                                     \
  TF_LITE_REPORT_ERROR(reporter, "%s took %d ticks (%d ms)", #func,     \
                                    duration_ticks, duration_ms);

#endif //TF_LITE_MICRO_EXECUTION_TIME

#ifndef TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START
#define TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(reporter)            \
  if (tflite::ticks_per_second() == 0) {                                \
    TF_LITE_REPORT_ERROR(reporter,                                      \
                         "no timer implementation found");              \
  }                                                                     \
  start_ticks = tflite::GetCurrentTimeTicks();
//#endif //TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START

//#ifndef TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END
#define TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(reporter, desc)        \
  duration_ticks = tflite::GetCurrentTimeTicks() - start_ticks;         \
  if (duration_ticks > INT_MAX / 1000) {                                \
    duration_ms = duration_ticks / (tflite::ticks_per_second() / 1000); \
  } else {                                                              \
    duration_ms = (duration_ticks * 1000) / tflite::ticks_per_second(); \
  }                                                                     \
  TF_LITE_REPORT_ERROR(reporter, "%s took %d ticks (%d ms)", desc,      \
                                    duration_ticks, duration_ms);
#endif //TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END

//error reporter for entire program
namespace pretask{
  static tflite::MicroErrorReporter task_error_reporter; 
  void pretask_setup();
}

/*
#ifdef __cplusplus
}
#endif //__cplusplus
*/
#endif