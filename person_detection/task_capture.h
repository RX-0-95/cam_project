#ifndef _TASK_CAPTURE_H_
#define _TASK_CAPUTER_H_

#include "detect_config.h"
#include "task_pretask.h"
#include "task_person_detection.h"
#include "image_provider.h"
#include "motion_detection.h"

#include "FreeRTOS.h"
#include "task.h"
#ifndef CAPTURE_FRAME_RATE
    #define CAPTURE_FRAME_RATE 4
#endif //CAPTURE_FRAME_RATE

namespace tcap{
    static uint8_t imageDat[96*96*2];
    static uint8_t imgeGreyDat[96*96];
    static int8_t prev_frame_data[kNumRows*kNumCols];
    static int8_t frame_diff_data[kNumRows*kNumCols];
    static MotionDetector * motion_detector = nullptr;
    static uint32_t pos_pixel_count = 0;
    static const uint8_t bitmapt_threshould = 32; 
    
    //static int8_t prev_frame_data
    static void
    yuv_capture_task(void *args)
    {
        static MotionDetector motion_detecto(pretask::input->data.int8,kNumRows,kNumCols,
                        prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                        DETECT_PERCENT_THRESHOLD);
        motion_detector = &motion_detecto;

        for(;;){
            GetYUVImage(pretask::error_reporter,imageDat);
            //detection motion
            bool has_motion = motion_detector->detect_motion();

            if (has_motion){
                TF_LITE_REPORT_ERROR(pretask::error_reporter,"===>Detect Motion!!!\n");
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

#endif//_TASK_CAPUTER_H_