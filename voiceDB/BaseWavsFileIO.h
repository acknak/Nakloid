#ifndef BaseWavsFileIO_h
#define BaseWavsFileIO_h

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>

#include "../PSOLA/BaseWav.h"
#include "../parser/WavParser.h"
#include "../parser/WavFormat.h"

namespace bwc {
  bool isBaseWavsContainerFile(std::string filename);
  BaseWavsContainer get(std::string filename);
  bool set(std::string filename, BaseWavsContainer *bwc);
};

#endif
