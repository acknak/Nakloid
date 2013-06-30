#ifndef UnitWaveformOverlapper_h
#define UnitWaveformOverlapper_h

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <list>
#include <string>
#include <vector>
#include <boost/filesystem/fstream.hpp>
#include "UnitWaveform.h"
#include "../Utilities.h"
#include "../parser/WavData.h"
#include "../parser/WavFormat.h"
#include "../parser/WavParser.h"

// Refference Object
class UnitWaveformOverlapper {
 public:
  UnitWaveformOverlapper(const WavFormat& format, const std::vector<float>& pitches);
  virtual ~UnitWaveformOverlapper();

  bool overlapping(const uw::UnitWaveformContainer* const uwc, long pron_start, long pron_end, const std::vector<short>& velocities);
  void outputNormalization();
  void outputCompressing();
  void outputWav(const std::wstring& output) const;
  void outputWav(const std::wstring& output, long ms_margin);

  // accessor
  const WavFormat& getWavFormat() const;
  const std::vector<long>& getPitchmarks() const;

 protected:
  std::vector<long>::const_iterator pos2it(long pos) const;

  long ms_margin;
  WavFormat format;
  std::vector<long> pitchmarks;
  std::vector<double> output_wav;

 private:
  UnitWaveformOverlapper(const UnitWaveformOverlapper& other);
  UnitWaveformOverlapper& operator=(const UnitWaveformOverlapper& other);
};

#endif
