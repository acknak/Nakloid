#ifndef WavData_h
#define WavData_h

#include <list>
#include <vector>
#include <cstring>
#include <iostream>

// Value Object
class WavData {
 public:
  WavData();
  explicit WavData(std::vector<short> data_vector);
  WavData(const short* data, long size);
  WavData(const WavData& other);
  ~WavData();

  WavData& operator = (const WavData& other);
  bool operator==(const WavData& other) const;
  bool operator!=(const WavData& other) const;
  const short* getData() const;
  std::list<short> getDataList();
  std::vector<short> getDataVector();
  void setData(const short* data, long size);
  void setData(std::list<short> data);
  void setData(std::vector<short> data);
  long getSize();

 private:
  void setSize(long size);
  short* data;
  long size;
};

#endif
