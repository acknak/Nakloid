#ifndef WavFormat_h
#define WavFormat_h

class WavFormat {
 public:
  WavFormat() : chunkSize(const_chunk_size) {};
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
  static bool isTag(char* tag);
  static bool isRiffTag(char* tag);
  static bool isWaveTag(char* tag);
  static bool isFmtTag(char* tag);
  static bool isFactTag(char* tag);
  static bool isDataTag(char* tag);
  static bool isListTag(char* tag);

  static const char riff[]; //riff header
  static const char wave[]; //wave header
  static const char fmt[]; //fmt chunk
  static const char fact[]; //fact chunk
  static const char data[]; //data chunk
  static const char list[]; //list chunk
};

#endif
