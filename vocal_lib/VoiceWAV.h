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
  VoiceWAV(const std::wstring& str_pron_alias, const boost::filesystem::path& path):Voice(str_pron_alias, path){}
  VoiceWAV(const PronunciationAlias& pron_alias, const boost::filesystem::path& path):Voice(pron_alias, path){}
  VoiceWAV(const VoiceWAV& other):Voice(other){}
  ~VoiceWAV(){}

  const UnitWaveformContainer* getUnitWaveformContainer() const;

 private:
  mutable Wav tmp_wav;
  static std::map< std::wstring, std::vector<double> > vowel_wav_map;

  const std::vector<double>& getVowelWav() const;
  const std::vector<double>& getPrefixVowelWav() const;
  void setVowelWav() const;
  inline double sinc(double x) const { return sin(M_PI*x) / (M_PI*x); }

  // inherit from WavHandler
  void chunkHeader(WavHeader wav_header);
  void chunkData(WavData wav_data);
};

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#endif
