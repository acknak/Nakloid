#ifndef Nakloid_h
#define Nakloid_h

#include <iostream>
#include <string>
#include <vector>
#include "core/UnitWaveformOverlapper.h"
#include "score/Score.h"
#include "vocal_lib/VocalLibrary.h"

#include <string>
#include <boost/filesystem/path.hpp>

class Nakloid {
 public:
  enum ScoreMode {
    score_mode_nak, score_mode_ust, score_mode_smf
  };
  enum PitchesMode {
    pitches_mode_pit, pitches_mode_lf0, pitches_mode_none
  };
  static struct Parameters {
    Parameters() {
      path_input_score = boost::filesystem::wpath(L"./input/score.nak");
      score_mode = score_mode_nak;
      path_lyrics = boost::filesystem::wpath(L"./input/lyrics.txt");
      path_input_pitches = boost::filesystem::wpath(L"./input/pitches.pit");
      pitch_mode = pitches_mode_none;
      path_singer = boost::filesystem::wpath(L"./vocal/voiceDB");
      path_prefix_map = boost::filesystem::wpath(L"");
      path_song = boost::filesystem::wpath(L"./output/song.wav");
      path_output_score = boost::filesystem::wpath(L"./output/score.nak");
      path_output_pitches = boost::filesystem::wpath(L"./output/pitches.pit");
      print_debug = false;
    }
      boost::filesystem::wpath path_input_score, path_lyrics, path_input_pitches, path_singer,
        path_prefix_map, path_song,path_output_score, path_output_pitches;
      enum ScoreMode score_mode;
      enum PitchesMode pitch_mode;
      bool print_debug;
  } params;

  Nakloid():score(0),vocal_lib(0) {}
  virtual ~Nakloid();

  bool vocalization();
  bool makeAllCache(bool save_pmp, bool save_uwc);
  bool makeCache(PronunciationAlias pron_alias, bool save_pmp, bool save_uwc);
  bool makeCache(std::wstring pron_alias, bool save_pmp, bool save_uwc);

 private:
  Nakloid(const Nakloid& other);
  Nakloid& operator=(const Nakloid& other);

  Score *score;
  VocalLibrary *vocal_lib;
};

#endif
