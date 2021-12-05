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
#ifndef _TASK_CAPTURE_TRANSFER_H
#define _TASK_CAPTURE_TRANSFER_H

#include "task_pretask.h"
#include "detect_config.h"
#include "image_provider.h"
#include "FreeRTOS.h"
#include "task.h"


/*
#ifdef __cplusplus
extern "C" {
#endif
*/

#ifndef CAPTURE_FRAME_RATE
    #define CAPTURE_FRAME_RATE 4
#endif

static void 
capture_transfer_task(void* args)
{
    tflite::ErrorReporter* error_repoter = &pretask::task_error_reporter;
    TickType_t capature_delay_ticks = pdMS_TO_TICKS((int32_t)1000/CAPTURE_FRAME_RATE);
    for(;;){
        TF_LITE_MICRO_EXECUTION_TIME_BEGIN
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_repoter);

        if (kTfLiteOk !=GetJPEGImageTransfer(error_repoter)){
            TF_LITE_REPORT_ERROR(error_repoter,"JPEG caputer failed\n");
        }
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_repoter,"JPEG capature and transfer\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/*
#ifdef __cplusplus
}
#endif //__cplusplus
*/
#endif //_TASK_CAPTURE_TRANSFER_H