#ifndef UnitWaveformMaker_h
#define UnitWaveformMaker_h

#include "../format/UnitWaveformContainer.h"

#include <vector>

class UnitWaveformMaker {
 public:
  static struct Parameters {
    Parameters() {
      target_rms = 0.05;
      num_lobes = 1;
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

  UnitWaveformMaker(UnitWaveformContainer *uwc, const std::vector<long>& pitchmarks)
   :uwc(uwc),pitchmarks(pitchmarks),sub_ovrl(0),sub_fade_start(0),sub_fade_end(0){}
  virtual ~UnitWaveformMaker(){}

  bool makeUnitWaveform(const std::vector<double>& voice, bool is_vcv);
  bool makeUnitWaveform(const std::vector<double>& voice, short pitch, bool is_vcv);

  // accessor
  UnitWaveformContainer getUnitWaveformContainer() const;
  const std::vector<long>& getPitchMarks() const;
  void setOvrl(long ms_ovrl, unsigned long fs);
  void setOvrl(long sub_ovrl);
  void setFadeParams(long ms_fade_start, long ms_fade_end, unsigned long fs);
  void setFadeParams(long sub_fade_start, long sub_fade_end);

 private:
  UnitWaveformMaker(const UnitWaveformMaker& other);
  UnitWaveformMaker& operator=(const UnitWaveformMaker& other);

  UnitWaveform makeUnitWaveform(short point, short pitch);
  UnitWaveform makeUnitWaveform(short point, short pitch, double scale);

  UnitWaveformContainer *uwc;
  std::vector<double> voice;
  std::vector<long> pitchmarks;
  long sub_ovrl;
  long sub_fade_start, sub_fade_end;
};

#endif
