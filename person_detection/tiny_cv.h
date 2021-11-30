/**
 * @file tiny_cv.cpp
 * @author Deyu Yang
 * @brief provide simple computer vison operations for 
 * grey scale image only. 
 * @version 0.1
 * @date 2021-11-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _TINY_CV_
#define _TINY_CV_

#include <stdint.h>

class TinyImage{
    public:
    uint8_t* img_data;
    uint32_t img_w;
    uint32_t img_h;
    uint32_t img_len;
    TinyImage(uint32_t img_w,uint32_t img_h);
    TinyImage(uint8_t* in_img_data, uint32_t img_w, uint32_t img_h);
    //TODO: assigne constructor
    //TODO: copy construcor
    ~TinyImage();

};
uint8_t* dilate(TinyImage* img); 

uint8_t* erode(TinyImage* img); 

uint8_t* gaussian(TinyImage* img);

uint8_t* sobel(TinyImage* img);

uint32_t bit_map_transfer(uint8_t* src_img,uint32_t src_w, uint32_t src_h, 
                        TinyImage* tg_img,uint8_t threshold);

#endif