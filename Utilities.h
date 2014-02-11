#ifndef Utilities_h
#define Utilities_h

#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <boost/assign.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "./parser/WavFormat.h"

namespace nak {
  enum ScoreMode {
    score_mode_nak, score_mode_ust, score_mode_smf
  };
  enum PitchesMode {
    pitches_mode_pit, pitches_mode_lf0, pitches_mode_none
  };
  class VoiceAlias {
   public:
    VoiceAlias():prefix(L""),pron(L""),suffix(L""){}
    explicit VoiceAlias(const std::wstring& alias);
    VoiceAlias(const std::wstring& prefix, const std::wstring& pron, const std::wstring& suffix):prefix(prefix),pron(pron),suffix(suffix){}
    bool checkVCV() const;
    std::wstring getAliasString() const;
    std::wstring getVowel() const;
    std::wstring getPrefixVowel() const;
    std::wstring prefix;
    std::wstring pron;
    std::wstring suffix;
  };

  // Input
  extern std::wstring path_singer;
  extern std::wstring path_prefix_map;

  extern enum ScoreMode score_mode;
  extern std::wstring path_nak;
  extern std::wstring path_ust;
  extern std::wstring path_smf;
  extern short track;
  extern std::wstring path_lyrics;

  extern enum PitchesMode pitches_mode;
  extern std::wstring path_pitches;
  extern unsigned char pitch_frame_length;

  // Output
  extern long margin;
  extern std::wstring path_song;
  extern std::wstring path_output_nak;
  extern std::wstring path_output_pit;

  // Nakloid
  extern bool cache;
  extern bool print_log;
  extern bool auto_vowel_combining;
  extern double vowel_combining_volume;

  // PitchMarker
  extern short pitch_margin;
  extern double xcorr_threshold;

  // UnitWaveformMaker
  extern double target_rms;
  extern unsigned char unit_waveform_lobe;
  extern bool uwc_normalize;

  // UnitWaveformOverlapper
  extern double fade_stretch;
  extern bool interpolation;
  extern bool overlap_normalize;
  extern bool wav_normalize;
  extern double max_volume;
  extern bool compressor;
  extern double threshold;
  extern double ratio;

  // Note
  extern short ms_front_padding;
  extern short ms_back_padding;

  // PitchArranger
  extern bool vibrato;
  extern bool overshoot;
  extern bool preparation;
  extern bool finefluctuation;
  extern short ms_overshoot;
  extern double pitch_overshoot;
  extern short ms_preparation;
  extern double pitch_preparation;
  extern short ms_vibrato_offset;
  extern short ms_vibrato_width;
  extern double pitch_vibrato;
  extern double finefluctuation_deviation;

  // Nakloid.ini parser
  extern bool parse(const std::wstring& path_ini);

  // tools
  extern WavFormat getDefaultFormat();
  extern long ms2pos(long ms, WavFormat format);
  extern long pos2ms(long pos, WavFormat format);
  extern long tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo);
  extern std::pair<bool, double> val2dB(double wav_value);
  extern double dB2val(const std::pair<bool, double>& dB);
  extern double cent2rate(const double cent);

  extern std::vector<double> normalize(const std::vector<double>& wav, double target_rms);
  extern std::vector<double> normalize(const std::vector<double>& wav, double target_mean, double target_var);
  extern std::vector<double> normalize(const std::vector<double>& wav, short target_max, short target_min);
  extern double getRMS(const std::vector<double>& wav);
  extern double getRMS(const std::vector<double>::const_iterator from, const std::vector<double>::const_iterator to);
  extern double getMean(const std::vector<double>& wav);
  extern double getMean(const std::vector<double>::const_iterator from, const std::vector<double>::const_iterator to);
  extern double getVar(const std::vector<double>& wav);
  extern double getVar(const std::vector<double>::const_iterator from, const std::vector<double>::const_iterator to);

  extern std::vector<double> getWindow(long length, unsigned char lobe);
  extern void multipleWindow(std::vector<double>::iterator from, const std::vector<double>::iterator to, unsigned char lobe);
  extern double sinc(double x);

  extern std::map<std::wstring, std::wstring>::const_iterator getVow2PronIt(const std::wstring& pron);

  // internal parameters
  extern std::map<std::wstring, std::wstring> vow2pron;
  extern std::map<std::wstring, std::wstring> pron2vow;
}

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
