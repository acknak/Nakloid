#ifndef UnitWaveformMaker_h
#define UnitWaveformMaker_h

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "core-inl.h"
#include "../format/UnitWaveformContainer.h"
#include "../format/Wav.h"

class UnitWaveformMaker {
 public:
  static struct Parameters {
    Parameters() {
      target_rms = 0.05;
      num_lobes = 3;
      normalize = true;
      min_repeat_length = 100;
      repeat_threshold = 0.98;
    };
    double target_rms;
    short num_lobes;
    bool normalize;
    short min_repeat_length;
    double repeat_threshold;
  } params;

  UnitWaveformMaker():sub_rep_start(0),sub_ovrl(0),sub_fade_start(0){}
  virtual ~UnitWaveformMaker(){}

  bool makeUnitWaveform(const std::vector<double>& voice, bool is_vcv);
  bool makeUnitWaveform(const std::vector<double>& voice, short pitch, bool is_vcv);

  // accessor
  const std::vector<UnitWaveform>& getUnitWaveform() const;
  const std::vector<long>& getPitchMarks() const;
  void setPitchMarks(const std::vector<long>& pitch_marks);
  void setPitchMarks(const std::vector<long>& pitch_marks, long ms_rep_start, unsigned long fs);
  void setPitchMarks(const std::vector<long>& pitch_marks, long ms_rep_start, long ms_ovrl, unsigned long fs);
  long getFadeStartSub() const;

 private:
  UnitWaveformMaker(const UnitWaveformMaker& other);
  UnitWaveformMaker& operator=(const UnitWaveformMaker& other);

  UnitWaveform makeUnitWaveform(short point, short pitch);
  UnitWaveform makeUnitWaveform(short point, short pitch, double scale);

  std::vector<UnitWaveform> unit_waveforms;
  std::vector<double> voice;
  std::vector<long> pitchmarks;
  long sub_rep_start;
  long sub_ovrl;
  long sub_fade_start;
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
