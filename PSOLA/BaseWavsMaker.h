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
  long getRepStartPoint();
  void setRepStart(long ms_rep_start, unsigned long fs);

 private:
  BaseWavsMaker(const BaseWavsMaker& other);
  BaseWavsMaker& operator=(const BaseWavsMaker& other);

  BaseWav makeBaseWav(int point);
  BaseWav makeBaseWav(int point, double scale);

  std::vector<BaseWav> base_wavs;
  std::vector<short> voice;
  std::vector<long> pitch_marks;
  long pos_rep_start;
  unsigned char lobe;
  long sub_rep_start;
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
