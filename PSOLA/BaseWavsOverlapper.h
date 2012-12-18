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
  BaseWavsOverlapper(WavFormat format, std::list<double> pitches);
  BaseWavsOverlapper(WavFormat format, std::vector<double> pitches);
  virtual ~BaseWavsOverlapper();

  bool overlapping(unsigned long ms_start, unsigned long ms_end, BaseWavsContainer bwc, std::vector<unsigned char> velocities);
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

  WavFormat format;
  std::vector<unsigned long> pitchmarks;
  std::vector<long> output_wav;
};

#endif
