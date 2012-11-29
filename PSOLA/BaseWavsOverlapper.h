#ifndef BaseWavsOverlapper_h
#define BaseWavsOverlapper_h

#include <list>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "BaseWav.h"
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
  void debug(std::string output);

  // accessor
  WavFormat getWavFormat();
  std::list<unsigned long> getPitchmarksList();
  std::vector<unsigned long> getPitchmarksVector();

 private:
  BaseWavsOverlapper(const BaseWavsOverlapper& other);
  BaseWavsOverlapper& operator=(const BaseWavsOverlapper& other);

  unsigned long ms2pos(unsigned long ms);
  unsigned long pos2ms(unsigned long pos);
  std::vector<unsigned long>::iterator pos2it(unsigned long pos);

  WavFormat format;
  std::vector<unsigned long> pitchmarks;
  std::vector<short> output_wav;
};

#endif
