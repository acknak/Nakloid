#ifndef PitchArranger_h
#define PitchArranger_h

#include <cmath>
#include <vector>
#include <boost/utility.hpp>
#include "Arranger.h"
#include "../Score.h"
#include "../parser/WavFormat.h"

class PitchArranger : Arranger {
 public:
  const static unsigned short overshoot_length;
  const static double overshoot_height;
  const static unsigned short preparation_length;
  const static double preparation_height;
  const static unsigned short vibrato_offset;
  const static unsigned short vibrato_width;
  const static double vibrato_depth;

  static void arrange(Score *score);

 private:
  static void vibrato(std::vector<double>::iterator it_pitches_begin, std::vector<double>::iterator it_pitches_end);
  static void overshoot(std::vector<double>::iterator it_pitches_begin, std::vector<double>::iterator it_pitches_end, double target_pitch);
  static void preparation(std::vector<double>::iterator it_pitches_begin, std::vector<double>::iterator it_pitches_end, double target_pitch);
  static void interpolation(std::vector<double>::iterator it_pitches, unsigned long ms_pron_start, unsigned long ms_pron_end, double target_pitch);
};

#endif
