#ifndef UnitWaveformOverlapper_h
#define UnitWaveformOverlapper_h

#include <list>
#include <cmath>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "UnitWaveform.h"
#include "../Utilities.h"
#include "../parser/WavFormat.h"
#include "../parser/WavData.h"
#include "../parser/WavParser.h"

// Refference Object
class UnitWaveformOverlapper {
 public:
  UnitWaveformOverlapper(WavFormat format, std::list<float> pitches);
  UnitWaveformOverlapper(WavFormat format, std::vector<float> pitches);
  virtual ~UnitWaveformOverlapper();

  bool overlapping(const uw::UnitWaveformContainer* uwc, long pron_start, long pron_end, std::vector<short> velocities);
  void outputWav(std::string output);
  void outputWav(std::string output, long ms_margin);

  // accessor
  WavFormat getWavFormat();
  std::list<long> getPitchmarksList();
  std::vector<long> getPitchmarksVector();

 private:
  UnitWaveformOverlapper(const UnitWaveformOverlapper& other);
  UnitWaveformOverlapper& operator=(const UnitWaveformOverlapper& other);

  std::vector<long>::iterator pos2it(long pos);

  long ms_margin;
  WavFormat format;
  std::vector<long> pitchmarks;
  std::vector<double> output_wav;
};

#endif
