#ifndef _MOTION_DETECTION_H_
#define _MOTION_DETECTION_H_

#include <stdint.h>



class MotionDetector{
    public:
        
        int8_t* image_data; 
        int32_t image_w;
        int32_t image_h;
        uint8_t bitmap_threshold;
        float percent_threshold;
        int8_t* prev_frame_data;
        int8_t* frame_diff_data;
        MotionDetector(int8_t* image_data, 
                        int image_width,
                        int image_height,
                        int8_t* prev_frame_data,
                        int8_t* frame_diff_data,
                        uint8_t bitmap_threshold,
                        float percent_threshold);
        int8_t* frame_difference(); 
        bool detect_motion();
        int8_t* apply();
    private:
        uint32_t pixel_count_threshold;
        uint32_t pixel_counter; 
        int32_t diff_pixel;
       
};


#endif //_MOTION_DETECT_H_