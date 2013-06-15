#ifndef Utilities_h
#define Utilities_h

#include <map>
#include <list>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/assign.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "./parser/WavFormat.h"

namespace nak {
  // General
  extern enum ScoreMode{score_mode_nak, score_mode_ust, score_mode_smf} score_mode;
  extern enum PitchesMode{pitches_mode_pit, pitches_mode_lf0, pitches_mode_none} pitches_mode;
  extern std::string path_pitches;
  extern std::string path_singer;
  extern std::string path_song;
  extern long margin;
  extern unsigned char pitch_frame_length;

  // General nak
  extern std::string path_nak;

  // General ust
  extern std::string path_ust;

  // General smf
  extern short track;
  extern std::string path_smf;
  extern std::string path_lyrics;

  // Nakloid
  extern bool cache;
  extern std::string path_log;
  extern bool auto_vowel_combining;
  extern double vowel_combining_volume;
  extern std::string path_output_nak;
  extern std::string path_output_pit;
  extern std::string path_prefix_map;

  // PitchMarker
  extern short pitch_margin;

  // UnitWaveformMaker
  extern short target_rms;
  extern unsigned char unit_waveform_lobe;
  extern bool is_normalize;

  // UnitWaveformOverlapper
  extern double fade_stretch;
  extern bool compressor;
  extern double threshold_x;
  extern double threshold_y;
  extern double max_volume;

  // Note
  extern short ms_front_edge;
  extern short ms_back_edge;

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
  extern bool interpolation;
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
  extern double getDB(long wav_value);
  extern short reverseDB(double db);

  extern std::vector<double> getHann(long len);
  extern std::vector<double> getTri(long len);
  extern std::vector<double> getLanczos(long len, unsigned char lobe);
  extern double sinc(double x);

  extern std::map<std::string, std::string>::const_iterator getVow2PronIt(std::string pron);
}

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
