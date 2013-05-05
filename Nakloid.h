#ifndef Nakloid_h
#define Nakloid_h

#include <map>
#include <list>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <boost/utility.hpp>
#include "Utilities.h"
#include "PitchArranger.h"
#include "voiceDB/Voice.h"
#include "voiceDB/VoiceDB.h"
#include "parser/WavFormat.h"
#include "parser/WavData.h"
#include "PSOLA/UnitWaveformOverlapper.h"
#include "parser/WavParser.h"
#include "score/Score.h"
#include "score/ScoreNAK.h"
#include "score/ScoreUST.h"
#include "score/ScoreSMF.h"

// Reference Object
class Nakloid {
 public:
  Nakloid();
  explicit Nakloid(nak::ScoreMode mode);
  virtual ~Nakloid();

  void loadDefaultFormat();
  bool loadScore(nak::ScoreMode mode);
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
