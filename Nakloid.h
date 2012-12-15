#ifndef Nakloid_h
#define Nakloid_h

#include <map>
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "Score.h"
#include "VoiceDB.h"
#include "Utilities.h"
#include "arranger/NoteArranger.h"
#include "arranger/PitchArranger.h"
#include "parser/WavFormat.h"
#include "parser/WavData.h"
#include "parser/WavParser.h"
#include "PSOLA/BaseWavsOverlapper.h"

// Reference Object
class Nakloid {
 public:
  Nakloid();
  explicit Nakloid(std::string path_ust);
  Nakloid(std::string singer, std::string path_smf, short track, std::string path_lyric, std::string path_song);
  virtual ~Nakloid();

  void setDefaultFormat();
  bool setScore(std::string path_ust);
  bool setScore(std::string singer, std::string path_smf, short track, std::string path_lyric, std::string path_song);
  bool vocalization();

  // accessor
  Score* getScore();
  WavFormat getFormat();
  void setFormat(WavFormat format);
  void setMargin(long margin);
  long getMargin();

 private:
  Nakloid(const Nakloid& other);
  Nakloid& operator=(const Nakloid& other);

  Score *score;
  VoiceDB *voice_db;
  WavFormat format;
  unsigned long margin;
};

#endif
