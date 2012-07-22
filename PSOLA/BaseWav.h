#ifndef BaseWav_h
#define BaseWav_h

#include <vector>

#include "../parser/WavFormat.h"
#include "../parser/WavData.h"

class BaseWavsFormat : public WavFormat {
 public:
  static const unsigned short BaseWavsFormatTag = 0xFFFF;
  static const short wAdditionalSize = 8;
  unsigned short wLobeSize;
  unsigned long dwRepeatStart;
  unsigned short wF0;
};

typedef struct {
  static const long chunkSize = 12;
  long dwPitchLeft;
  long dwPitchRight;
  long dwPosition;
} BaseWavFact;

typedef struct {
  BaseWavFact fact;
  WavData data;
} BaseWav;

typedef struct {
  BaseWavsFormat format;
  std::vector<BaseWav> data;
} BaseWavsContainer;

#endif
