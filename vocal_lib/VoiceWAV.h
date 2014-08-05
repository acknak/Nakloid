#ifndef VoiceWAV_h
#define VoiceWAV_h

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include "Voice.h"
#include "../core/PitchMarker.h"
#include "../core/UnitWaveformMaker.h"
#include "../format/PronunciationAlias.h"
#include "../format/UnitWaveformContainer.h"
#include "../parser/WavHandler.h"
#include "../parser/WavParser.h"

class VoiceWAV: public Voice, public WavHandler {
 public:
  explicit VoiceWAV(boost::filesystem::path path_wav):Voice(path_wav){}
  VoiceWAV(const VoiceWAV& other):Voice(other){}
  ~VoiceWAV(){}

  const UnitWaveformContainer* getUnitWaveformContainer() const;

private:
  mutable Wav tmp_wav;

  const std::vector<double>& getVowelWav() const;
  const bool hasVowelWav() const;

  // inherit from WavHandler
  void chunkHeader(WavHeader wav_header);
  void chunkData(WavData wav_data);
};

#endif
