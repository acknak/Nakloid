#ifndef UnitWaveformMaker_h
#define UnitWaveformMaker_h

#include <list>
#include <cmath>
#include <vector>
#include <numeric>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "UnitWaveform.h"
#include "../Utilities.h"
#include "../parser/WavData.h"

// Refference Object
class UnitWaveformMaker {
 public:
  UnitWaveformMaker();
  virtual ~UnitWaveformMaker();

  bool makeUnitWaveform(std::vector<short> voice, bool is_vcv);
  bool makeUnitWaveform(std::vector<short> voice, short pitch, bool is_vcv);

  // accessor
  std::vector<uw::UnitWaveform> getUnitWaveform();
  std::vector<long> getPitchMarkVector();
  std::list<long> getPitchMarkList();
  void setPitchMarks(std::vector<long> pitch_marks);
  void setPitchMarks(std::vector<long> pitch_marks, long ms_rep_start, unsigned long fs);
  void setPitchMarks(std::vector<long> pitch_marks, long ms_rep_start, long ms_ovrl, unsigned long fs);
  long UnitWaveformMaker::getRepStartSub();

 private:
  UnitWaveformMaker(const UnitWaveformMaker& other);
  UnitWaveformMaker& operator=(const UnitWaveformMaker& other);

  uw::UnitWaveform makeUnitWaveform(short point, short pitch);
  uw::UnitWaveform makeUnitWaveform(short point, short pitch, double scale);

  std::vector<uw::UnitWaveform> unit_waveforms;
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
