#ifndef UnitWaveform_h
#define UnitWaveform_h

#include <vector>

#include "../parser/WavFormat.h"
#include "../parser/WavData.h"

namespace uw {
  class UnitWaveformFormat : public WavFormat {
   public:
    static const long chunkSize = 30; // 16(linearPCM)+14(extension)
    static const unsigned short UnitWaveformFormatTag = 0xFFFF;
    static const short wAdditionalSize = 12;
    short wLobeSize;
    long dwRepeatStart;
    double wF0;
  };

  class UnitWaveformFact {
   public:
    static const long chunkSize = 12;
    long dwPitchLeft;
    long dwPitchRight;
    long dwPosition;
  };

  typedef struct {
    UnitWaveformFact fact;
    WavData data;
  } UnitWaveform;

  typedef struct {
    UnitWaveformFormat format;
    std::vector<UnitWaveform> unit_waveforms;
  } UnitWaveformContainer;
}

#endif
