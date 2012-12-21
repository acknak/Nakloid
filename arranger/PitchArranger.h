#ifndef PitchArranger_h
#define PitchArranger_h

#include <cmath>
#include <vector>
#include <boost/utility.hpp>
#include "../Score.h"
#include "../Utilities.h"
#include "../parser/WavFormat.h"

class PitchArranger {
 public:
  static void arrange(Score *score);

 private:
  static void vibrato(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end);
  static void overshoot(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end, float target_pitch);
  static void preparation(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end, float target_pitch);
  static void interpolation(std::vector<float>::iterator it_pitches, unsigned long ms_pron_start, unsigned long ms_pron_end, float target_pitch);
};

#endif
