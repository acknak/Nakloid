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

#define M_PI 3.1415926535897932384626433832795

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
  long getConsPos();
  void setConsPos(unsigned short cons, unsigned long fs);
  std::list<long> getMarkList();
  std::vector<long> getMarkVector();

 private:
  PitchMarker(const PitchMarker& other);
  PitchMarker& operator=(const PitchMarker& other);

  std::vector<float> xcorr(std::vector<short>::iterator it_start, std::vector<short>::iterator it_base, short exp_dist);
  std::vector<float> xcorr(std::vector<short>::iterator first, std::vector<short>::iterator last);
  short pitch;
  short win_size;
  long cons_pos;
  std::list<long> mark_list;
  std::vector<float> filter;
};

#endif
