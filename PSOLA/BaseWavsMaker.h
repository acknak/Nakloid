#ifndef BaseWavsMaker_h
#define BaseWavsMaker_h

#include <list>
#include <cmath>
#include <vector>
#include <numeric>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "BaseWav.h"
#include "../parser/WavData.h"

// Refference Object
class BaseWavsMaker {
 public:
  BaseWavsMaker();
  virtual ~BaseWavsMaker();

  bool makeBaseWavs();

  // accessor
  std::vector<BaseWav> getBaseWavs();
  std::vector<short> getVoice();
  void setVoice(std::vector<short> voice);
  std::vector<long> getPitchMarkVector();
  std::list<long> getPitchMarkList();
  void setPitchMarks(std::vector<long> pitch_marks);
  void setPitchMarks(std::list<long> pitch_marks);
  unsigned char getLobe();
  void setLobe(unsigned char lobe);
  long getRepStartPoint();
  void setRepStart(long rep_start_ms, unsigned long fs);

 private:
  BaseWavsMaker(const BaseWavsMaker& other);
  BaseWavsMaker& operator=(const BaseWavsMaker& other);

  std::vector<short> normalize(std::vector<short>wav, double target_rms);
  std::vector<short> normalize(std::vector<short>wav, double target_mean, double target_var);
  std::vector<short> normalize(std::vector<short>wav, short target_max, short target_min, double target_mean);
  double getRMS(std::vector<short> wav);
  double getMean(std::vector<short> wav);
  double getVar(std::vector<short> wav, double mean);

  std::vector<double> getHann(long len);
  std::vector<double> getTri(long len);
  std::vector<double> getLanczos(long len, unsigned short lobe);
  double sinc(double x);

  std::vector<BaseWav> base_wavs;
  std::vector<short> voice;
  std::vector<long> pitch_marks;
  unsigned char lobe;
  long rep_start;
  long rep_start_point;
  long rep_len_point;
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
