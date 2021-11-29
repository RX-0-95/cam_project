#ifndef _PERSONE_DETECTION_MODEL_SETTINGS_H_
#define _PERSONE_DETECTION_MODEL_SETTINGS_H_

constexpr int kNumCols = 96;
constexpr int kNumRows = 96;
constexpr int kNumChannels = 1;

constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;

constexpr int kCategoryCount = 2;
constexpr int kPersonIndex = 1;
constexpr int kNotAPersonIndex = 0;
extern const char* kCategoryLabels[kCategoryCount];

#endif //_PERSONE_DETECTION_MODEL_SETTINGS_H_