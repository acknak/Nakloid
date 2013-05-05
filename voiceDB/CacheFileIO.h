#ifndef CacheFileIO_h
#define CacheFileIO_h

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>

#include "../PSOLA/UnitWaveform.h"
#include "../parser/WavParser.h"
#include "../parser/WavFormat.h"

namespace uw {
  bool isUwcFile(std::string filename);
  uw::UnitWaveformContainer load(std::string filename);
  bool save(std::string filename, uw::UnitWaveformContainer *uwc);
};

#endif
