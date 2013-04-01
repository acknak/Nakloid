#ifndef VoiceDB_h
#define VoiceDB_h

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "parser/WavParser.h"
#include "PSOLA/PitchMarker.h"
#include "PSOLA/BaseWav.h"
#include "PSOLA/BaseWavsMaker.h"
#include "PSOLA/BaseWavsFileIO.h"

typedef struct {
  std::string path;
  std::string filename;
  short offs; // offset(left blank)
  short cons; // consonant part(unaltered range)
  short blnk; // blank(right blank)
  short prec; // preceding utterance
  short ovrl; // overlap range
  double frq;
  BaseWavsContainer bwc;
} Voice;

// Reference Object
class VoiceDB {
 public:
  VoiceDB();
  VoiceDB(std::string path_singer);
  virtual ~VoiceDB();

  bool initVoiceMap();
  bool initVoiceMap(std::string path_oto_ini);
  Voice getVoice(std::string pron);
  bool isPron(std::string pron);

  // accessor
  void setSingerPath(std::string path_singer);
  std::string getSingerPath();

 private:
  VoiceDB(const VoiceDB& other);
  VoiceDB& operator=(const VoiceDB& other);

  Voice getNullVoice();

  std::string path_singer;
  std::map<std::string, Voice> voice_map;
  //std::map<std::string, char> pron2vow;
};

#endif
