#ifndef UnitWaveform_h
#define UnitWaveform_h

#include <boost/filesystem.hpp>
#include "Wav.h"

class UnitWaveformHeader : public WavHeader {
 public:
  UnitWaveformHeader(){}
  bool operator==(const UnitWaveformHeader& other) const;
  bool operator!=(const UnitWaveformHeader& other) const;

  const static long const_chunk_size = 30; //16(linearPCM)+14(extension)
  const static unsigned short UnitWaveformFormatTag = 0xFFFF;
  const static short wAdditionalSize = 10; //short(2byte)+long(4byte)+float(4byte)
  short wLobeSize;
  long dwRepeatStart;
  float wF0;
};

class UnitWaveform {
 public:
  static const long chunkSize = 12;
  UnitWaveform(){}
  UnitWaveform(short dwPitchLeft, long dwPitchRight, float dwPosition)
    :dwPitchLeft(dwPitchLeft), dwPitchRight(dwPitchRight), dwPosition(dwPosition){}
  bool operator==(const UnitWaveform& other) const;
  bool operator!=(const UnitWaveform& other) const;

  const static long const_chunk_size = 12;
  long dwPitchLeft;
  long dwPitchRight;
  long dwPosition;
  WavData data;
};

class UnitWaveformContainer{
 public:
  UnitWaveformContainer(){}
  UnitWaveformContainer(UnitWaveformHeader header, std::vector<UnitWaveform> unit_waveforms)
    :header(header), unit_waveforms(unit_waveforms) {}
  bool operator==(const UnitWaveformContainer& other) const;
  bool operator!=(const UnitWaveformContainer& other) const;

  UnitWaveformHeader header;
  std::vector<UnitWaveform> unit_waveforms;

  void save(const boost::filesystem::path& path_uwc);
  static bool isUwcFormatFile(const boost::filesystem::path& path_uwc);
  void clear();
};

#endif
