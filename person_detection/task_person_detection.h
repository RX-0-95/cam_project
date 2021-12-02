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

//necessary setup for 
namespace{
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr; 

    constexpr int kTensorArenaSize = 136*1024;
    static uint8_t tensor_arena[kTensorArenaSize];

    MotionDetector * motion_detector = nullptr;
    static int8_t prev_frame_data[kNumRows*kNumCols];
    static int8_t frame_diff_data[kNumRows*kNumCols];
    //TinyCv related
    static TinyImage* tg_img = nullptr;
    static uint32_t pos_pixel_count = 0;
    static const uint8_t bitmap_threshold = 32;
}

static void 
person_detection_setup(void){
    
}

static void
person_detection_task(void *args __attribute__((unused))){
    //static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &pretask::task_error_reporter;
    //uint8_t tensor_arena[kTensorArenaSize];
    // map model into usable data structure
    model = tflite::GetModel(g_person_detect_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION){
        TF_LITE_REPORT_ERROR(error_reporter,
                            "Model provider is schema version %d not equal"
                            "to spport version %d.",
                            model->version(),TFLITE_SCHEMA_VERSION);
        return;
    }
    // pull in only the operation implementations we need
    static tflite::MicroMutableOpResolver<5> micro_op_resolver;
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();
    
    //Build interrpter to run the model with
    static tflite::MicroInterpreter static_interpreter(
        model,micro_op_resolver,tensor_arena,kTensorArenaSize,error_reporter);
    interpreter = &static_interpreter;

   // allocate memory from the tensor_arena for the model's tensor
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    // Get information about the memory area to use for the model's input.
    input = interpreter->input(0);
    TF_LITE_REPORT_ERROR(error_reporter,"Person detection setup clear!!!");

    // Detect motion
    //Allocate space for motion detector 

    static MotionDetector motion_detecto(input->data.int8,kNumRows,kNumCols,
                          prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                          DETECT_PERCENT_THRESHOLD);
    motion_detector = &motion_detecto;

    // TinyCv related
    static TinyImage tg_im(kNumRows,kNumCols);    
    tg_img = &tg_im;

    //task loop
    for(;;){
        //printf("Perform person detection !!!\n");

        //printf(uxTaskGetStackHighWaterMark(NULL));
        TF_LITE_MICRO_EXECUTION_TIME_BEGIN
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_reporter);

        // get image from provider
        if (kTfLiteOk != GetImage(error_reporter,kNumCols,kNumRows,kNumChannels,
            input->data.int8)){
                TF_LITE_REPORT_ERROR(error_reporter,"Image capture failed");
            }
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter,"GetImage")

        // motion detection
        bool has_motion = motion_detector->detect_motion();

        if (has_motion){
            TF_LITE_REPORT_ERROR(error_reporter,"===>Detect Motion!!!\n");
            // Run model on the input
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_reporter)
            if(kTfLiteOk != interpreter->Invoke()){
                TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
            }
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter, "Invoke")
            TfLiteTensor* output = interpreter->output(0);
            
            // process interfernce result
            int8_t person_score = output->data.uint8[kPersonIndex];
            int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
            RespondToDetection(error_reporter,person_score,no_person_score);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


/*
#ifdef __cplusplus
}
#endif
*/
#endif//_TASK_PERSON_DETECTION_H_