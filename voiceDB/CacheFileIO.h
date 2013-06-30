#ifndef CacheFileIO_h
#define CacheFileIO_h

#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <boost/filesystem/fstream.hpp>
#include "../parser/WavFormat.h"
#include "../parser/WavParser.h"
#include "../PSOLA/UnitWaveform.h"

namespace uw {
  bool isUwcFile(const std::wstring& filename);
  uw::UnitWaveformContainer load(const std::wstring& filename);
  bool save(const std::wstring& filename, const uw::UnitWaveformContainer& uwc);
};

#endif
