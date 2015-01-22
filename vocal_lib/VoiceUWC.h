#ifndef VoiceUWC_h
#define VoiceUWC_h

#include "Voice.h"
#include "../parser/WavHandler.h"

#include <string>
#include <boost/filesystem/path.hpp>

class PronunciationAlias;
class UnitWaveformContainer;

class VoiceUWC: public Voice, public WavHandler {
 public:
  VoiceUWC(const std::wstring& str_pron_alias, const boost::filesystem::path& path):Voice(str_pron_alias, path){}
  VoiceUWC(const PronunciationAlias& pron_alias, const boost::filesystem::path& path):Voice(pron_alias, path){}
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
