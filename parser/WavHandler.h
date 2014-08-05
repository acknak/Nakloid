#ifndef WavHandler_h
#define WavHandler_h

#include "../format/Wav.h"

class WavHandler {
 public:
  WavHandler(){}
  virtual ~WavHandler(){}

  virtual void chunkHeader(WavHeader wav_header){}
  virtual void chunkHeaderAdditionalField(short chunk_size, const unsigned char* const data){}
  virtual void chunkData(WavData wav_data){}
  virtual void chunkList(long chunk_size, const unsigned char* const data){}
  virtual void chunkFact(long chunk_size, const unsigned char* const data){}

 private:
  WavHandler(const WavHandler& other);
  WavHandler& operator=(const WavHandler& other);
};

#endif
