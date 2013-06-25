#ifndef Utilities_h
#define Utilities_h

#include <map>
#include <list>
#include <cmath>
#include <tuple>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/assign.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "./parser/WavFormat.h"

namespace nak {
  enum ScoreMode {
    score_mode_nak, score_mode_ust, score_mode_smf
  };
  enum PitchesMode{
    pitches_mode_pit, pitches_mode_lf0, pitches_mode_none
  };

  // Input
  extern std::string path_singer;
  extern std::string path_prefix_map;

  extern enum ScoreMode score_mode;
  extern std::string path_nak;
  extern std::string path_ust;
  extern std::string path_smf;
  extern short track;
  extern std::string path_lyrics;

  extern enum PitchesMode pitches_mode;
  extern std::string path_pitches;
  extern unsigned char pitch_frame_length;

  // Output
  extern long margin;
  extern std::string path_song;
  extern std::string path_output_nak;
  extern std::string path_output_pit;

  // Nakloid
  extern bool cache;
  extern std::string path_log;
  extern bool auto_vowel_combining;
  extern double vowel_combining_volume;

  // PitchMarker
  extern short pitch_margin;

  // UnitWaveformMaker
  extern double target_rms;
  extern unsigned char unit_waveform_lobe;
  extern bool is_normalize;

  // UnitWaveformOverlapper
  extern double fade_stretch;
  extern bool interpolation;
  extern double max_volume;
  extern bool compressor;
  extern double threshold;
  extern double ratio;

  // Note
  extern short ms_front_padding;
  extern short ms_back_padding;

  // PitchArranger
  extern short ms_overshoot;
  extern double pitch_overshoot;
  extern short ms_preparation;
  extern double pitch_preparation;
  extern short ms_vibrato_offset;
  extern short ms_vibrato_width;
  extern double pitch_vibrato;
  extern double finefluctuation_deviation;
  extern bool vibrato;
  extern bool overshoot;
  extern bool preparation;
  extern bool completion;
  extern bool finefluctuation;

  // internal parameters
  extern std::map<std::string, std::string> vow2pron;
  extern std::map<std::string, std::string> pron2vow;

  // Nakloid.ini parser
  extern bool parse(std::string path_ini);

  // tools
  extern long ms2pos(long ms, WavFormat format);
  extern long pos2ms(long pos, WavFormat format);
  extern long tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo);

  extern std::vector<double> normalize(std::vector<double>wav, double target_rms);
  extern std::vector<double> normalize(std::vector<double>wav, double target_mean, double target_var);
  extern std::vector<double> normalize(std::vector<double>wav, short target_max, short target_min);
  extern double getRMS(std::vector<double> wav);
  extern double getMean(std::vector<double> wav);
  extern double getVar(std::vector<double> wav, double mean);
  extern std::pair<bool, double> val2dB(double wav_value);
  extern double dB2val(std::pair<bool, double> dB);

  extern std::vector<double> getHann(long len);
  extern std::vector<double> getTri(long len);
  extern std::vector<double> getLanczos(long len, unsigned char lobe);
  extern double sinc(double x);

  extern std::map<std::string, std::string>::const_iterator getVow2PronIt(std::string pron);
  extern std::tuple<std::string, std::string, std::string, bool> parseAlias(std::string alias);
}

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
