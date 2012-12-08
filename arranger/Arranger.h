#ifndef Arranger_h
#define Arranger_h

#include <vector>
#include <cmath>
#include "../parser/WavFormat.h"

class Arranger {
 public:
  static unsigned long ms2pos(unsigned long ms, WavFormat format);
  static unsigned long pos2ms(unsigned long pos, WavFormat format);
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
