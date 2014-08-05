#ifndef VoiceUWC_h
#define VoiceUWC_h

#include "VocalLibrary.h"
#include "Voice.h"
#include "../format/UnitWaveformContainer.h"
#include "../parser/WavHandler.h"
#include "../utilities/Tools.h"

class VoiceUWC: public Voice, public WavHandler {
 public:
  explicit VoiceUWC(boost::filesystem::path path_uwc):Voice(path_uwc){}
  VoiceUWC(const VoiceUWC& other):Voice(other){}
  ~VoiceUWC();

  const UnitWaveformContainer* getUnitWaveformContainer() const;

 private:
  // inherit from WavHandler
  void chunkHeader(WavHeader wav_header);
  void chunkHeaderAdditionalField(short chunk_size, const unsigned char* const data);
  void chunkData(WavData wav_data);
  void chunkFact(long chunk_size, const unsigned char* const data);
};

#endif
