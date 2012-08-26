#ifndef BaseWavsOverlapper_h
#define BaseWavsOverlapper_h

#include <list>
#include <vector>
#include <string>
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
  void setPitchMarks(std::list<long> pitch_marks);
  void setPitchMarks(std::vector<long> pitch_marks);
  std::list<long> getPitchMarkList();
  std::vector<long> getPitchMarkVector();
  void setBaseWavs(std::vector<BaseWav> base_wavs);
  std::vector<BaseWav> getBaseWavs();
  std::list<short> getOutputWavList();
  std::vector<short> getOutputWavVector();
  void setRepStart(long rep_start);
  long getRepStart();
  void setVelocity(unsigned short velocity);
  void setVelocity(double velocity);
  double getVelocity();
  void isNormalize(bool is_normalize);
  bool isNormalize();

 private:
  BaseWavsOverlapper(const BaseWavsOverlapper& other);
  BaseWavsOverlapper& operator=(const BaseWavsOverlapper& other);
  std::vector<short> normalize(std::vector<short>wav, double target_rms);
  double getRMS(std::vector<short> wav);

  std::vector<short> output_wav;
  std::vector<long> pitch_marks;
  std::vector<BaseWav> base_wavs;
  long rep_start;
  double velocity;
  bool is_normalize;
};

#endif
