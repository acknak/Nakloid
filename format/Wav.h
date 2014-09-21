#ifndef Wav_h
#define Wav_h

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

class WavHeader {
 public:
  WavHeader():
    wFormatTag(1), //linear
    wChannels(1), //monoral
    dwSamplesPerSec(44100),
    dwAvgBytesPerSec(88200),
    wBlockAlign(2),
    wBitsPerSamples(16)
  {}
  ~WavHeader(){}
  WavHeader(const WavHeader& other)
    :wFormatTag(other.wFormatTag),wChannels(other.wChannels),dwSamplesPerSec(other.dwSamplesPerSec),
     dwAvgBytesPerSec(other.dwAvgBytesPerSec),wBlockAlign(other.wBlockAlign),wBitsPerSamples(other.wBitsPerSamples){}
  WavHeader& operator=(const WavHeader& other);
  bool operator==(const WavHeader& other) const;
  bool operator!=(const WavHeader& other) const;

  static const long const_chunk_size = 16;
  uint16_t wFormatTag;
  uint16_t wChannels;
  uint32_t dwSamplesPerSec;
  uint32_t dwAvgBytesPerSec;
  uint16_t wBlockAlign;
  uint16_t wBitsPerSamples;

  // tag
  static const char tag_riff[];
  static const char tag_wave[];
  static const char tag_fmt_[];
  static bool isRiffTag(const char* const tag);
  static bool isWaveTag(const char* const tag);
  static bool isFmtTag(const char* const tag);
};

class WavData {
 public:
  WavData(){}
  WavData(const WavData& other):data(other.data){}
  explicit WavData(const std::vector<double>& data):data(data){}
  WavData(std::vector<double>::const_iterator from, std::vector<double>::const_iterator to):data(std::vector<double>(from,to)){}
  WavData(const int16_t data[], long chunk_size);
  ~WavData(){}
  WavData& operator=(const WavData& other);
  bool operator==(const WavData& other) const;
  bool operator!=(const WavData& other) const;

  void clear();

  // accessor
  std::vector<int16_t> getDataForWavFile() const;
  const std::vector<double>& getData() const;
  void setData(const std::vector<double>& data);
  void setData(const int16_t* const data, uint32_t chunk_size);
  long getSize() const;
  long getSizeForWavFile() const;
  double getRMS() const;

  // tag
  static bool isFactTag(const char* const tag);
  static bool isDataTag(const char* const tag);
  static bool isListTag(const char* const tag);
  static const char tag_fact[];
  static const char tag_data[];
  static const char tag_list[];

 private:
  std::vector<double> data;
};

class Wav{
 public:
  Wav(){}
  Wav(const Wav& other):header(other.header),data(other.data){}
  Wav(WavHeader header, WavData data):header(header),data(data){}
  Wav(WavHeader header, std::vector<double> data):header(header),data(data){}
  Wav(WavHeader header, const short* const data, long chunk_size):header(header),data(data, chunk_size){}
  ~Wav(){}
  Wav& operator=(const Wav& other);
  bool operator==(const Wav& other) const;
  bool operator!=(const Wav& other) const;

  WavHeader header;
  WavData data;
  void save(const boost::filesystem::path& path);
  void clear();
};

#endif
