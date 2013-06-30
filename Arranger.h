#ifndef Arranger_h
#define Arranger_h

#include <cmath>
#include <list>
#include <vector>
#include <boost/random.hpp>
#include "parser/WavFormat.h"
#include "score/Note.h"
#include "score/Score.h"
#include "voiceDB/voice.h"
#include "voiceDB/voiceDB.h"
#include "Utilities.h"

class Arranger {
 public:
  static void arrange(VoiceDB* voice_db, Score* score);

 protected:
  static VoiceDB* voice_db;
  static Score* score;

  static void checkAlias(std::list<Note>::iterator it_notes);
  static void loadParamsFromVoiceDB(std::list<Note>::iterator it_notes, const Voice* voice);

  static void vibrato(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end);
  static void overshoot(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end, float target_pitch);
  static void preparation(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end, float target_pitch);
  static void completion(std::vector<float>::iterator it_pitches, long ms_pron_start, long ms_pron_end, float target_pitch);
};

#endif
