#ifndef Nakloid_h
#define Nakloid_h

#include <map>
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "Score.h"
#include "VoiceDB.h"
#include "arranger/PitchArranger.h"
#include "arranger/DataArranger.h"
#include "parser/WavFormat.h"
#include "parser/WavData.h"
#include "parser/WavParser.h"
#include "PSOLA/BaseWavsOverlapper.h"

// Reference Object
class Nakloid {
 public:
  Nakloid();
  Nakloid(std::string singer, std::string path_score, short track, std::string path_lyric);
  virtual ~Nakloid();

  void setDefaultFormat();
  bool setScorePath(std::string path_score, short track, std::string path_lyric);
  bool vocalization();
  bool vocalization(std::string path_song);

  // accessor
  WavFormat getFormat();
  void setFormat(WavFormat format);
  std::vector<Note> getNotes();
  void setSongPath(std::string path_song);
  std::string getSongPath();
  void setSinger(std::string singer);
  std::string getSinger();
  void setMargin(long margin);
  long getMargin();

 private:
  Nakloid(const Nakloid& other);
  Nakloid& operator=(const Nakloid& other);

  double ms2pos(long ms);

  VoiceDB *voice_db;
  WavFormat format;
  std::vector<Note> notes;
  std::string path_song;
  long margin;
};

#endif
