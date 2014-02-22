#ifndef UnitWaveformMaker_h
#define UnitWaveformMaker_h

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "UnitWaveform.h"
#include "../Utilities.h"
#include "../parser/WavData.h"

// Refference Object
class UnitWaveformMaker {
 public:
  UnitWaveformMaker();
  virtual ~UnitWaveformMaker();

  bool makeUnitWaveform(const std::vector<double>& voice, bool is_vcv);
  bool makeUnitWaveform(const std::vector<double>& voice, short pitch, bool is_vcv);

  // accessor
  const std::vector<uw::UnitWaveform>& getUnitWaveform() const;
  const std::vector<long>& getPitchMarks() const;
  void setPitchMarks(const std::vector<long>& pitchmarks);
  void setPitchMarks(const std::vector<long>& pitchmarks, long ms_rep_start, unsigned long fs);
  void setPitchMarks(const std::vector<long>& pitchmarks, long ms_rep_start, long ms_ovrl, unsigned long fs);
  long UnitWaveformMaker::getFadeStartSub() const;

 protected:
  uw::UnitWaveform makeUnitWaveform(short point, short pitch);
  uw::UnitWaveform makeUnitWaveform(short point, short pitch, double scale);

  std::vector<uw::UnitWaveform> unit_waveforms;
  std::vector<double> voice;
  std::vector<long> pitchmarks;
  unsigned char lobe;
  long sub_rep_start;
  long sub_ovrl;

 private:
  UnitWaveformMaker(const UnitWaveformMaker& other);
  UnitWaveformMaker& operator=(const UnitWaveformMaker& other);
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
