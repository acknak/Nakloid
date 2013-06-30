#ifndef Nakloid_h
#define Nakloid_h

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <string>
#include <vector>
#include <boost/utility.hpp>
#include "Arranger.h"
#include "parser/WavData.h"
#include "parser/WavFormat.h"
#include "parser/WavParser.h"
#include "PSOLA/UnitWaveformOverlapper.h"
#include "score/Score.h"
#include "score/ScoreNAK.h"
#include "score/ScoreSMF.h"
#include "score/ScoreUST.h"
#include "Utilities.h"
#include "voiceDB/Voice.h"
#include "voiceDB/VoiceDB.h"

class Nakloid {
 public:
  Nakloid();
  explicit Nakloid(nak::ScoreMode mode);
  virtual ~Nakloid();

  void loadDefaultFormat();
  bool loadScore(nak::ScoreMode mode);
  bool vocalization();

  // accessor
  Score* getScore() const;
  const WavFormat& getFormat() const;
  void setFormat(const WavFormat& format);
  void setMargin(long margin);
  long getMargin() const;

 protected:
  Score *score;
  VoiceDB *voice_db;
  WavFormat format;
  long margin;

 private:
  Nakloid(const Nakloid& other);
  Nakloid& operator=(const Nakloid& other);
};

#endif
