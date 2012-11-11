#ifndef PitchArranger_h
#define PitchArranger_h

#include <vector>
#include <cmath>

class PitchArranger {
 public:
  const static short overshoot_length;
  const static double overshoot_height;
  const static short preparation_length;
  const static double preparation_height;
  const static short vibrato_offset;
  const static short vibrato_width;
  const static double vibrato_depth;

  static void vibrato(std::vector<double> *guide_pitches);
  static void overshoot(std::vector<double> *guide_pitches, double pitch_from, double pitch_to);
  static void preparation(std::vector<double> *guide_pitches, double pitch_from, double pitch_to);
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
