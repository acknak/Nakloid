#ifndef BaseWavsFileIO_h
#define BaseWavsFileIO_h

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>

#include "BaseWav.h"
#include "../parser/WavParser.h"
#include "../parser/WavFormat.h"

// Refference Object
class BaseWavsFileIO {
 public:
  BaseWavsFileIO();
  virtual ~BaseWavsFileIO();

  bool isBaseWavsContainerFile(std::string filename);
  BaseWavsContainer get(std::string filename);
  bool set(std::string filename, BaseWavsContainer *bwc);

 private:
  BaseWavsFileIO(const BaseWavsFileIO& other);
  BaseWavsFileIO& operator=(const BaseWavsFileIO& other);
};

#endif
