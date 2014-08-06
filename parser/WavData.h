#ifndef WavData_h
#define WavData_h

#include <cstring>
#include <iostream>
#include <vector>
#include "WavParser.h"

class WavParser;

class WavData {
 public:
  WavData();
  explicit WavData(const std::vector<double>& data_vector);
  WavData(const short* const data, long chunk_size);
  WavData(const WavData& other);
  virtual ~WavData();

  // accessor
  std::vector<short> getWavData() const;
  const std::vector<double>& getData() const;
  void setData(const short* const data, long chunk_size);
  void setData(const std::vector<double>& data);
  long getSize() const;
  double getRMS() const;

 protected:
  std::vector<double> data;
};

#endif
