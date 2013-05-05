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
  void outputWav(std::string output, unsigned long ms_margin);

  // accessor
  WavFormat getWavFormat();
  std::list<unsigned long> getPitchmarksList();
  std::vector<unsigned long> getPitchmarksVector();

 private:
  UnitWaveformOverlapper(const UnitWaveformOverlapper& other);
  UnitWaveformOverlapper& operator=(const UnitWaveformOverlapper& other);

  std::vector<unsigned long>::iterator pos2it(unsigned long pos);

  long ms_margin;
  WavFormat format;
  std::vector<unsigned long> pitchmarks;
  std::vector<long> output_wav;
};

#endif
