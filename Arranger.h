#ifndef Arranger_h
#define Arranger_h

#include <list>
#include <cmath>
#include <vector>
#include <boost/random.hpp>
#include "Utilities.h"
#include "voiceDB/voice.h"
#include "voiceDB/voiceDB.h"
#include "score/Note.h"
#include "score/Score.h"
#include "parser/WavFormat.h"

class Arranger {
 public:
  static void arrange(VoiceDB* voice_db, Score* score);

 private:
  static VoiceDB* voice_db;
  static Score* score;

  static void checkAlias(std::list<Note>::iterator it_notes);
  static void loadParamsFromVoiceDB(std::list<Note>::iterator it_notes, const Voice* voice);
  static void checkProxy(std::list<Note>::iterator it_notes, const Voice* proxy_voice);

  static void vibrato(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end);
  static void overshoot(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end, float target_pitch);
  static void preparation(std::vector<float>::iterator it_pitches_begin, std::vector<float>::iterator it_pitches_end, float target_pitch);
  static void interpolation(std::vector<float>::iterator it_pitches, unsigned long ms_pron_start, unsigned long ms_pron_end, float target_pitch);
};

#endif
