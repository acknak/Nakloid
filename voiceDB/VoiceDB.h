#ifndef VoiceDB_h
#define VoiceDB_h

#include <map>
#include <tuple>
#include <cctype>
#include <string>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "Voice.h"
#include "../parser/WavParser.h"
#include "../PSOLA/PitchMarker.h"
#include "../PSOLA/UnitWaveformMaker.h"
class Voice;

// Reference Object
class VoiceDB {
 public:
  VoiceDB();
  VoiceDB(std::string path_singer);
  virtual ~VoiceDB();

  bool initVoiceMap();
  bool initVoiceMap(std::string path_oto_ini);
  const Voice* getVoice(std::string alias);
  bool isAlias(std::string alias);
  bool isVowel(std::string pron);
  std::vector<double> getVowel(std::string alias);

  // accessor
  void setSingerPath(std::string path_singer);
  std::string getSingerPath();

 private:
  VoiceDB(const VoiceDB& other);
  VoiceDB& operator=(const VoiceDB& other);

  std::string path_singer;
  std::map<std::string, Voice> voice_map;
  std::map<std::string, std::vector<double>> vowel_map;
};

#endif
