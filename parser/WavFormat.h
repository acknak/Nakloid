#ifndef WavFormat_h
#define WavFormat_h

class WavFormat {
 public:
  long chunkSize;
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

  bool operator==(const WavFormat& other) const;
  bool operator!=(const WavFormat& other) const;
};

#endif
