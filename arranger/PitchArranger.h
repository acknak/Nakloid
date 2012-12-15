#ifndef PitchArranger_h
#define PitchArranger_h

#include <cmath>
#include <vector>
#include <boost/utility.hpp>
#include "Arranger.h"
#include "../Score.h"
#include "../Parameters.h"
#include "../parser/WavFormat.h"

class PitchArranger : Arranger {
 public:
  static void arrange(Score *score);

 private:
  static void vibrato(std::vector<double>::iterator it_pitches_begin, std::vector<double>::iterator it_pitches_end);
  static void overshoot(std::vector<double>::iterator it_pitches_begin, std::vector<double>::iterator it_pitches_end, double target_pitch);
  static void preparation(std::vector<double>::iterator it_pitches_begin, std::vector<double>::iterator it_pitches_end, double target_pitch);
  static void interpolation(std::vector<double>::iterator it_pitches, unsigned long ms_pron_start, unsigned long ms_pron_end, double target_pitch);
};

#endif
