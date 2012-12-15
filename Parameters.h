#ifndef Params_h
#define Params_h

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

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
}

#endif
