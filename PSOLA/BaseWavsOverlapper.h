#ifndef BaseWavsOverlapper_h
#define BaseWavsOverlapper_h

#include <list>
#include <cmath>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "BaseWav.h"
#include "../Utilities.h"
#include "../parser/WavFormat.h"
#include "../parser/WavData.h"
#include "../parser/WavParser.h"

// Refference Object
class BaseWavsOverlapper {
 public:
  BaseWavsOverlapper(WavFormat format, std::list<float> pitches);
  BaseWavsOverlapper(WavFormat format, std::vector<float> pitches);
  virtual ~BaseWavsOverlapper();

  bool overlapping(const BaseWavsContainer* bwc, long pron_start, long pron_end, std::vector<short> velocities);
  void outputWav(std::string output);
  void outputWav(std::string output, unsigned long ms_margin);

  // accessor
  WavFormat getWavFormat();
  std::list<unsigned long> getPitchmarksList();
  std::vector<unsigned long> getPitchmarksVector();

 private:
  BaseWavsOverlapper(const BaseWavsOverlapper& other);
  BaseWavsOverlapper& operator=(const BaseWavsOverlapper& other);

  std::vector<unsigned long>::iterator pos2it(unsigned long pos);

  long ms_margin;
  WavFormat format;
  std::vector<unsigned long> pitchmarks;
  std::vector<long> output_wav;
};

#endif
