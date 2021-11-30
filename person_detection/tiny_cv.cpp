#include "tiny_cv.h"
#include "string.h"
#include "assert.h"

TinyImage::TinyImage(uint32_t img_w, uint32_t img_h):
img_w(img_w),img_h(img_h){
    img_len = img_w*img_h;
    this->img_data = new uint8_t[img_len];
}

TinyImage::TinyImage(uint8_t* in_img_data, uint32_t img_w, uint32_t img_h):
img_w(img_w),img_h(img_h){
    img_len = img_w*img_h;
    this->img_data = new uint8_t[img_w*img_h];
    memcpy(img_data,in_img_data,img_len);
}

TinyImage::~TinyImage(){
    delete [] this->img_data;
}

uint32_t bit_map_transfer(uint8_t* src_img,uint32_t src_w, uint32_t src_h, 
                        TinyImage* tg_img,uint8_t threshold){
    assert (src_w == tg_img->img_w);
    assert (src_h == tg_img->img_h);
    uint32_t pos_count = 0;
    for (uint32_t i=0; i<tg_img->img_len;i++){
       if (src_img[i]>=threshold){
           tg_img->img_data[i] = 1;
           pos_count += 1;
       }
       else{
           tg_img->img_data[i] = 0;
       }
    }
    return pos_count;
}