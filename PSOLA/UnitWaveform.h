#ifndef UnitWaveform_h
#define UnitWaveform_h

#include <vector>

#include "../parser/WavFormat.h"
#include "../parser/WavData.h"

namespace uw {
  class UnitWaveformFormat : public WavFormat {
   public:
    UnitWaveformFormat() : WavFormat(const_chunk_size) {}
    explicit UnitWaveformFormat(long chunkSize) : WavFormat(chunkSize) {}
    virtual ~UnitWaveformFormat() {}
    UnitWaveformFormat& operator=(const WavFormat& other){
      WavFormat::operator=(other);return *this;
    }
    UnitWaveformFormat& operator=(const UnitWaveformFormat& other){
      WavFormat::operator=(other);wLobeSize=other.wLobeSize;dwRepeatStart=other.dwRepeatStart;wF0=other.wF0;return *this;
    }

    static const long const_chunk_size = 30; //16(linearPCM)+14(extension)
    static const unsigned short UnitWaveformFormatTag = 0xFFFF;
    static const short wAdditionalSize = 10; //short(2byte)+long(4byte)+float(4byte)
    short wLobeSize;
    long dwRepeatStart;
    float wF0;
  };

  class UnitWaveformFact {
   public:
    explicit UnitWaveformFact() : chunkSize(const_chunk_size) {};
    explicit UnitWaveformFact(long chunkSize) : chunkSize(chunkSize) {};
    virtual ~UnitWaveformFact(){}
    UnitWaveformFact& operator=(const UnitWaveformFact& other){
      dwPitchLeft=other.dwPitchLeft;dwPitchRight=other.dwPitchRight;dwPosition=other.dwPosition;return *this;
    }

    static const long const_chunk_size = 12;
    const long chunkSize;
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
