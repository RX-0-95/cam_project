/**
 * @file motion_detection.cpp
 * @author Deyu Yang
 * @brief class for detect motion for grey scale image only
 * @version 0.1
 * @date 2021-11-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "motion_detection.h"
#include <cstdlib>
#include <string.h>
#include <iostream>
MotionDetector::MotionDetector(int8_t* p_image_data,
                            int p_image_width, 
                            int p_image_height,
                            int8_t* p_prev_frame_data,
                            int8_t* p_frame_diff_data):
image_data(p_image_data),
image_w(p_image_width),
image_h(p_image_height),
prev_frame_data(p_prev_frame_data),
frame_diff_data(p_frame_diff_data)
{}

int8_t* MotionDetector::apply()
{
    //apply frame abs difference to the curren frame
    for (int32_t i=0;i<image_w*image_h;i++){
        frame_diff_data[i] = abs(prev_frame_data[i]-image_data[i]);
    }
    //update prev_frame_data
    memcpy(this->prev_frame_data,this->image_data,sizeof(int8_t)*image_h*image_w);
    //TODO: add cv operation blur, erode, dilate
    
    return frame_diff_data;
}


