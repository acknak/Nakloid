#ifndef WavFormat_h
#define WavFormat_h

#include <cstddef>

class WavFormat {
 public:
  WavFormat() : chunkSize(const_chunk_size) {
    setDefaultValues();
  };
  explicit WavFormat(long chunkSize) : chunkSize(chunkSize) {};
  virtual ~WavFormat(){}
  WavFormat& operator=(const WavFormat& other);

  static const long const_chunk_size = 16;
  const long chunkSize;
  short wFormatTag;
  unsigned short wChannels;
  unsigned long dwSamplesPerSec;
  unsigned long dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSamples;

  void setDefaultValues();
  static bool isTag(const char* const tag);
  static bool isRiffTag(const char* const tag);
  static bool isWaveTag(const char* const tag);
  static bool isFmtTag(const char* const tag);
  static bool isFactTag(const char* const tag);
  static bool isDataTag(const char* const tag);
  static bool isListTag(const char* const tag);

  static const char riff[]; //riff header
  static const char wave[]; //wave header
  static const char fmt[]; //fmt chunk
  static const char fact[]; //fact chunk
  static const char data[]; //data chunk
  static const char list[]; //list chunk
};

#endif
