#ifndef WavData_h
#define WavData_h

#include <vector>
#include <cstring>
#include <iostream>

// Value Object
class WavData {
 public:
  WavData();
  explicit WavData(std::vector<double> data_vector);
  WavData(const short* data, long size);
  WavData(const WavData& other);
  ~WavData();

  WavData& operator = (const WavData& other);
  bool operator==(const WavData& other) const;
  bool operator!=(const WavData& other) const;
  const short* getData() const;
  std::vector<double> getDataVector() const;
  void setData(const short* data, long size);
  void setData(std::vector<double> data);
  long getWavDataSize() const;

 private:
  std::vector<double> data;
};

#endif
