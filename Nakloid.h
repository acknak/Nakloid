#ifndef Nakloid_h
#define Nakloid_h

#include <cstdio>
#include <iostream>
#include <locale>
#include <string>
#include <vector>
#include <boost/assign.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "core/UnitWaveformOverlapper.h"
#include "score/Score.h"
#include "score/ScoreNAK.h"
#include "score/ScoreSMF.h"
#include "score/ScoreUST.h"
#include "vocal_lib/Voice.h"
#include "vocal_lib/VocalLibrary.h"

class Nakloid {
 public:
  Nakloid(std::wstring path_ini);
  virtual ~Nakloid();

  bool vocalization();
  bool is_logging();

 private:
  Nakloid(const Nakloid& other);
  Nakloid& operator=(const Nakloid& other);

  Score *score;
  VocalLibrary *vocal_lib;

  enum ScoreMode {
    score_mode_nak, score_mode_ust, score_mode_smf
  };
  enum PitchesMode {
    pitches_mode_pit, pitches_mode_lf0, pitches_mode_none
  };

  // Input
  boost::filesystem::path path_input_score;
  enum ScoreMode score_mode;
  std::wstring path_lyrics;
  std::wstring path_input_pitches;
  enum PitchesMode pitch_mode;
  std::wstring path_singer;
  std::wstring path_prefix_map;

  // Output
  std::wstring path_song;
  std::wstring path_output_score;
  std::wstring path_output_pitches;
  bool print_log;
  bool print_debug;
};

#endif
