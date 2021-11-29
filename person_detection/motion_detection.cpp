
#include "motion_detection.h"
#include <cstdlib>
#include <string.h>
MotionDetector::MotionDetector(int8_t* image_data,
                            int image_width, 
                            int image_height,
                            int8_t* prev_frame_data,
                            int8_t* frame_diff_data):
image_data(image_data),image_w(image_width),
image_h(image_height),prev_frame_data(prev_frame_data),
frame_diff_data(frame_diff_data)
{}

int8_t* MotionDetector::apply()
{
    //apply frame abs difference to the curren frame
    for (int32_t i=0;i<image_w*image_h;i++){
        frame_diff_data[i] = abs(image_data[i] - prev_frame_data[i]);
    }

    //update prev_frame_data
    memcpy(prev_frame_data,image_data,image_h*image_w);
    return frame_diff_data;
}




