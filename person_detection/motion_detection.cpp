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
#include "detect_config.h"
#include "tiny_cv.h"

#include <cstdlib>
#include <string.h>
#include <iostream>
#include <climits>

MotionDetector::MotionDetector(int8_t* p_image_data,
                            int p_image_width, 
                            int p_image_height,
                            int8_t* p_prev_frame_data,
                            int8_t* p_frame_diff_data,
                            uint8_t p_bitmap_threshold,
                            float p_percent_threshold):
image_data(p_image_data),
image_w(p_image_width),
image_h(p_image_height),
prev_frame_data(p_prev_frame_data),
frame_diff_data(p_frame_diff_data),
bitmap_threshold(p_bitmap_threshold),
percent_threshold(p_percent_threshold)
{
    pixel_count_threshold = (uint32_t)image_w*image_h*percent_threshold;
    pixel_counter = 0;
    diff_pixel = 0;
}

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

int8_t* MotionDetector::frame_difference()
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

bool MotionDetector::detect_motion()
{
    pixel_counter = 0;
    //apply frame abs difference to the curren frame
    for (int32_t i=0;i<image_w*image_h;i++){
        frame_diff_data[i] = abs(prev_frame_data[i]-image_data[i]);
        diff_pixel = abs(prev_frame_data[i]-image_data[i]);
        if (diff_pixel >= bitmap_threshold) pixel_counter += 1;
        
        //TODO: consider add cliff for uint8 data
        frame_diff_data[i] = diff_pixel;
    }
    //update prev_frame_data
    memcpy(this->prev_frame_data,this->image_data,sizeof(int8_t)*image_h*image_w);
    return (pixel_counter >= pixel_count_threshold) ? true:false;
}