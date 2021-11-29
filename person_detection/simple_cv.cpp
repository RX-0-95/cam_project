/**
 * @file simple_cv.cpp
 * @author Deyu Yang
 * @brief provide simple computer vison operations for 
 * grey scale image only. 
 * @version 0.1
 * @date 2021-11-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _SIMPLE_CV_
#define _SIMPLE_CV_

#include <stdint.h>

int8_t* dilate(); 

int8_t* erode(); 

int8_t* gaussian();

int8_t* sobel();

#endif