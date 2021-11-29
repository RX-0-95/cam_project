#ifndef _MOTION_DETECTION_H_
#define _MOTION_DETECTION_H_

#include <stdint.h>



class MotionDetector{
    public:
        
        int8_t* image_data; 
        int32_t image_w;
        int32_t image_h;
        MotionDetector(int8_t* image_data, 
                        int image_width,
                        int image_height,
                        int8_t* prev_frame_data,
                        int8_t* frame_diff_data);
        int8_t* apply();
        int8_t* prev_frame_data;
        int8_t* frame_diff_data;
    private:
       
};


#endif //_MOTION_DETECT_H_