#ifndef VoiceDB_h
#define VoiceDB_h

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "parser/WavParser.h"
#include "PSOLA/PitchMarker.h"
#include "PSOLA/BaseWav.h"
#include "PSOLA/BaseWavsMaker.h"
#include "PSOLA/BaseWavsFileIO.h"

typedef struct {
  std::string filename;
  unsigned short offs; // offset(left blank)
  unsigned short cons; // consonant part(unaltered range)
  unsigned short blnk; // blank(right blank)
  unsigned short prec; // preceding utterance
  unsigned short ovrl; // overlap range
  double frq;
  BaseWavsContainer bwc;
} Voice;

// Reference Object
class VoiceDB {
 public:
  VoiceDB();
  VoiceDB(std::string singer);
  virtual ~VoiceDB();

  bool initVoiceMap();
  bool initVoiceMap(std::string filename);
  Voice getVoice(std::string pron);

  // accessor
  void setSinger(std::string singer);
  std::string getSinger();

 private:
  VoiceDB(const VoiceDB& other);
  VoiceDB& operator=(const VoiceDB& other);

  Voice getNullVoice();

  std::string singer;
  std::map<std::string, Voice> voice_map;
};

#endif
