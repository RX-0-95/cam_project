/**
 * @file task_person_detection.h
 * @author Deyu Yang
 * @brief FreeRtos task for person detection. Assume all
 * usart connect is taking care of before the task
 * @version 0.1
 * @date 2021-12-01
 * 
 * @copyright Copyright (c) 2021
 * 
*/

#ifndef _TASK_PERSON_DETECTION_H_
#define _TASK_PERSON_DETECTION_H_



#include "task_pretask.h"
#include "detect_config.h"
#include "model_settings.h"
#include "person_detection_model_data.h"

#include "image_provider.h"
#include "motion_detection.h"
#include "detection_responder.h"
#include "tiny_cv.h"

#include <climits>

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

/*
extern TfLiteTensor* pretask::input;
extern tflite::ErrorReporter* pretask::error_reporter;
extern const tflite::Model * pretask::model;
extern tflite::MicroInterpreter* pretask::interpreter;
*/
//necessary setup for 


/*
#ifdef __cplusplus
}
#endif
*/
#endif//_TASK_PERSON_DETECTION_H_