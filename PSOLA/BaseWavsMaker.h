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
#include "../Utilities.h"
#include "../parser/WavData.h"

// Refference Object
class BaseWavsMaker {
 public:
  BaseWavsMaker();
  virtual ~BaseWavsMaker();

  bool makeBaseWavs(std::vector<short> voice, bool is_vcv);
  bool makeBaseWavs(std::vector<short> voice, short pitch, bool is_vcv);

  // accessor
  std::vector<BaseWav> getBaseWavs();
  std::vector<long> getPitchMarkVector();
  std::list<long> getPitchMarkList();
  void setPitchMarks(std::vector<long> pitch_marks);
  void setPitchMarks(std::vector<long> pitch_marks, long ms_rep_start, unsigned long fs);
  void setPitchMarks(std::vector<long> pitch_marks, long ms_rep_start, long ms_ovrl, unsigned long fs);
  long BaseWavsMaker::getRepStartSub();

 private:
  BaseWavsMaker(const BaseWavsMaker& other);
  BaseWavsMaker& operator=(const BaseWavsMaker& other);

  BaseWav makeBaseWav(short point, short pitch);
  BaseWav makeBaseWav(short point, short pitch, double scale);

  std::vector<BaseWav> base_wavs;
  std::vector<short> voice;
  std::vector<long> pitch_marks;
  unsigned char lobe;
  long sub_rep_start;
  long sub_ovrl;
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
