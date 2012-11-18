#ifndef BaseWavsOverlapper_h
#define BaseWavsOverlapper_h

#include <list>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "BaseWav.h"
#include "../parser/WavFormat.h"
#include "../parser/WavData.h"
#include "../parser/WavParser.h"

// Refference Object
class BaseWavsOverlapper {
 public:
  BaseWavsOverlapper();
  virtual ~BaseWavsOverlapper();

  bool overlapping();
  void debugTxt(std::string output);
  void debugWav(std::string output);

  // accessor
  void setPitchMarks(std::list<unsigned long> pitch_marks);
  void setPitchMarks(std::vector<unsigned long> pitch_marks);
  std::list<unsigned long> getPitchMarkList();
  std::vector<unsigned long> getPitchMarkVector();
  void setBaseWavs(std::vector<BaseWav> base_wavs);
  std::vector<BaseWav> getBaseWavs();
  std::list<short> getOutputWavList();
  std::vector<short> getOutputWavVector();
  void setRepStart(unsigned long rep_start);
  unsigned long getRepStart();
  void setVelocity(unsigned short velocity);
  void setVelocity(double velocity);
  double getVelocity();

 private:
  BaseWavsOverlapper(const BaseWavsOverlapper& other);
  BaseWavsOverlapper& operator=(const BaseWavsOverlapper& other);

  std::vector<short> output_wav;
  std::vector<unsigned long> pitch_marks;
  std::vector<BaseWav> base_wavs;
  unsigned long rep_start;
  double velocity;
};

#endif
