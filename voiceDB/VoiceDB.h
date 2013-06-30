#ifndef VoiceDB_h
#define VoiceDB_h

#include <cctype>
#include <map>
#include <string>
#include <tuple>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "Voice.h"
#include "../parser/WavParser.h"
#include "../PSOLA/PitchMarker.h"
#include "../PSOLA/UnitWaveformMaker.h"
class Voice;

class VoiceDB {
 public:
  VoiceDB();
  VoiceDB(const std::wstring& path_singer);
  virtual ~VoiceDB();

  bool initVoiceMap();
  bool initVoiceMap(const std::wstring& path_oto_ini);

  // accessor
  const Voice* getVoice(const std::wstring& alias) const;
  bool isAlias(const std::wstring& alias) const;
  bool isVowel(const std::wstring& subject) const;
  const std::vector<double>& getVowel(const std::wstring& subject) const;
  void setSingerPath(const std::wstring& path_singer);
  const std::wstring& getSingerPath() const;

 protected:
  std::wstring path_singer;
  std::map<std::wstring, Voice> voice_map;
  std::map<std::wstring, std::vector<double>> vowel_map;

 private:
  VoiceDB(const VoiceDB& other);
  VoiceDB& operator=(const VoiceDB& other);
};

#endif
