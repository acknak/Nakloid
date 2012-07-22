#ifndef PitchMarker_h
#define PitchMarker_h

#include <list>
#include <cmath>
#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
 
#include "fftw3.h"
#include "../parser/WavData.h"

// Refference Object
class PitchMarker {
 public:
  PitchMarker();
  explicit PitchMarker(short win_size);
  PitchMarker(short pitch, unsigned long fs);
  virtual ~PitchMarker();

  bool mark(WavData input);
  bool mark(std::list<short> input);
  bool mark(std::vector<short> input);
  void debug(std::string output);

  // accessor
  short getPitch();
  void setPitch(short pitch, unsigned long fs);
  short getWinSize();
  void setWinSize(short win_size);
  std::list<long> getMarkList();
  std::vector<long> getMarkVector();

 private:
  PitchMarker(const PitchMarker& other);
  PitchMarker& operator=(const PitchMarker& other);

  std::vector<float> xcorr(std::vector<short>::iterator first, std::vector<short>::iterator last);
  short pitch;
  short win_size;
  std::list<long> mark_list;
};

#endif
