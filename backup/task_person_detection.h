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
    //error_reporter = &pretask::task_error_reporter;
    pretask::model = tflite::GetModel(g_person_detect_model_data);
    if (pretask::model->version() != TFLITE_SCHEMA_VERSION){
        TF_LITE_REPORT_ERROR(pretask::error_reporter,
                            "Model provider is schema version %d not equal"
                            "to spport version %d.",
                            pretask::model->version(),TFLITE_SCHEMA_VERSION);
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
        pretask::model,micro_op_resolver,pretask::tensor_arena,pretask::kTensorArenaSize,pretask::error_reporter);
    pretask::interpreter = &static_interpreter;

    // allocate memory from the tensor_arena for the model's tensor
    TfLiteStatus allocate_status = pretask::interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(pretask::error_reporter, "AllocateTensors() failed");
        return;
    }
    pretask::input = pretask::interpreter->input(0);
    TF_LITE_REPORT_ERROR(pretask::error_reporter,"Person detection setup clear!!!");
}

static void
person_detection_task(void *args __attribute__((unused))){
    person_detection_setup();    
    static MotionDetector motion_detecto(pretask::input->data.int8,kNumRows,kNumCols,
                          prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                          DETECT_PERCENT_THRESHOLD);
    
    /*
    static MotionDetector motion_detecto(pretask::model_input->data.int8,kNumRows,kNumCols,
                        prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                        DETECT_PERCENT_THRESHOLD);
    */

    motion_detector = &motion_detecto;
    // TinyCv related
    static TinyImage tg_im(kNumRows,kNumCols);    
    tg_img = &tg_im;

    //task loop
    for(;;){
        TF_LITE_MICRO_EXECUTION_TIME_BEGIN
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(pretask::error_reporter);

        // get image from provider
        if (kTfLiteOk != GetImage(pretask::error_reporter,kNumCols,kNumRows,kNumChannels,
            pretask::input->data.int8)){
                TF_LITE_REPORT_ERROR(pretask::error_reporter,"Image capture failed");
            }
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(pretask::error_reporter,"GetImage");

        // motion detection
        bool has_motion = motion_detector->detect_motion();

        if (has_motion){
            TF_LITE_REPORT_ERROR(pretask::error_reporter,"===>Detect Motion!!!\n");
            // Run model on the input
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(pretask::error_reporter)
            if(kTfLiteOk != pretask::interpreter->Invoke()){
                TF_LITE_REPORT_ERROR(pretask::error_reporter, "Invoke failed.");
            }
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(pretask::error_reporter, "Invoke")
            TfLiteTensor* output = pretask::interpreter->output(0);
            
            // process interfernce result
            int8_t person_score = output->data.uint8[kPersonIndex];
            int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
            RespondToDetection(pretask::error_reporter,person_score,no_person_score);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


/*
#ifdef __cplusplus
}
#endif
*/
#endif//_TASK_PERSON_DETECTION_H_


namespace{
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
    //error_reporter = &pretask::task_error_reporter;
    

    pretask::model = tflite::GetModel(g_person_detect_model_data);
    
    if (pretask::model->version() != TFLITE_SCHEMA_VERSION){
        TF_LITE_REPORT_ERROR(pretask::error_reporter,
                           "Model provider is schema version %d not equal"
                           "to spport version %d.",
                        pretask::model->version(),TFLITE_SCHEMA_VERSION);
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
        pretask::model,micro_op_resolver,pretask::tensor_arena,pretask::kTensorArenaSize,pretask::error_reporter);
    pretask::interpreter = &static_interpreter;


    // allocate memory from the tensor_arena for the model's tensor

    TfLiteStatus allocate_status = pretask::interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(pretask::error_reporter, "AllocateTensors() failed");
        return;
    }
    pretask::input = pretask::interpreter->input(0);
    TF_LITE_REPORT_ERROR(pretask::error_reporter,"Person detection setup clear!!!");

}

static void
person_detection_task(void *args __attribute__((unused))){
    //person_detection_setup();    
    static MotionDetector motion_detecto(pretask::input->data.int8,kNumRows,kNumCols,
                          prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                          DETECT_PERCENT_THRESHOLD);
    
    /*
    static MotionDetector motion_detecto(pretask::model_input->data.int8,kNumRows,kNumCols,
                        prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                        DETECT_PERCENT_THRESHOLD);
    */

    motion_detector = &motion_detecto;
    // TinyCv related
    static TinyImage tg_im(kNumRows,kNumCols);    
    tg_img = &tg_im;

    //task loop
    for(;;){
        TF_LITE_MICRO_EXECUTION_TIME_BEGIN
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(pretask::error_reporter);

        // get image from provider
        if (kTfLiteOk != GetImage(pretask::error_reporter,kNumCols,kNumRows,kNumChannels,
            pretask::input->data.int8)){
                TF_LITE_REPORT_ERROR(pretask::error_reporter,"Image capture failed");
            }
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(pretask::error_reporter,"GetImage");

        // motion detection
        bool has_motion = motion_detector->detect_motion();

        if (has_motion){
            TF_LITE_REPORT_ERROR(pretask::error_reporter,"===>Detect Motion!!!\n");
            // Run model on the input
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(pretask::error_reporter)
            if(kTfLiteOk != pretask::interpreter->Invoke()){
                TF_LITE_REPORT_ERROR(pretask::error_reporter, "Invoke failed.");
            }
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(pretask::error_reporter, "Invoke")
            TfLiteTensor* output = pretask::interpreter->output(0);
            
            // process interfernce result
            int8_t person_score = output->data.uint8[kPersonIndex];
            int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
            RespondToDetection(pretask::error_reporter,person_score,no_person_score);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

