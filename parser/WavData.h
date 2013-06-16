#ifndef WavData_h
#define WavData_h

#include <vector>
#include <cstring>
#include <iostream>

#include "WavParser.h"
class WavParser;

// Value Object
class WavData {
 public:
  WavData();
  explicit WavData(std::vector<double> data_vector);
  WavData(const short* data, long chunk_size);
  WavData(const WavData& other);
  virtual ~WavData();

  std::vector<short> getWavData() const;
  std::vector<double> getData() const;
  std::vector<double>::const_iterator getDataIterator() const;
  void setData(const short* data, long chunk_size);
  void setData(std::vector<double> data);
  long getSize() const;

 protected:
  std::vector<double> data;
};

#endif
