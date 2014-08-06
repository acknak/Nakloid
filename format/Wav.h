#ifndef Wav_h
#define Wav_h

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
  short wFormatTag;
  unsigned short wChannels;
  unsigned long dwSamplesPerSec;
  unsigned long dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSamples;

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
  WavData(const short data[], long chunk_size);
  ~WavData(){}
  WavData& operator=(const WavData& other);
  bool operator==(const WavData& other) const;
  bool operator!=(const WavData& other) const;

  void clear();

  // accessor
  std::vector<short> getDataForWav() const;
  const std::vector<double>& getData() const;
  void setData(const std::vector<double>& data);
  void setData(const short* const data, long chunk_size);
  long getSize() const;
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
