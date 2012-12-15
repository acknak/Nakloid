#ifndef Utilities_h
#define Utilities_h

#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "./parser/WavFormat.h"

namespace nak {
  // Nakloid
  extern enum ScoreMode{score_mode_ust, score_mode_smf, score_mode_nak} score_mode;
  extern std::string singer;
  extern std::string path_ust;
  extern std::string path_smf;
  extern short track;
  extern std::string path_lyric;
  extern std::string path_song;
  extern unsigned long margin;
  extern bool cache;

  // PitchMarker
  extern unsigned short pitch_margin;

  // BaseWavsMaker
  extern unsigned char base_wavs_lobe;

  // NoteArranger
  extern unsigned short ms_front_edge;
  extern unsigned short ms_back_edge;
  extern bool sharpen_front;
  extern bool sharpen_back;

  // PitchArranger
  extern unsigned short ms_overshoot;
  extern double pitch_overshoot;
  extern unsigned short ms_preparation;
  extern double pitch_preparation;
  extern unsigned short ms_vibrato_offset;
  extern unsigned short ms_vibrato_width;
  extern double pitch_vibrato;
  extern bool vibrato;
  extern bool overshoot;
  extern bool preparation;
  extern bool interpolation;

  // parser
  extern void parse(std::string path_ini);
  template <class T>
  extern void ptree2var(T *var, boost::property_tree::ptree ptree, std::string path);

  // tools
  extern unsigned long ms2pos(unsigned long ms, WavFormat format);
  extern unsigned long pos2ms(unsigned long pos, WavFormat format);

  extern std::vector<short> normalize(std::vector<short>wav, double target_rms);
  extern std::vector<short> normalize(std::vector<short>wav, double target_mean, double target_var);
  extern std::vector<short> normalize(std::vector<short>wav, short target_max, short target_min);
  extern double getRMS(std::vector<short> wav);
  extern double getMean(std::vector<short> wav);
  extern double getVar(std::vector<short> wav, double mean);

  extern std::vector<double> getHann(long len);
  extern std::vector<double> getTri(long len);
  extern std::vector<double> getLanczos(long len, unsigned short lobe);
  extern double sinc(double x);
}

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
