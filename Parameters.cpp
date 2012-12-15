#include "Parameters.h"

using namespace std;

namespace nak {
  // Nakloid
  enum ScoreMode score_mode = score_mode_ust;
  std::string path_ust = "score.ust";
  std::string path_smf = "score.mid";
  std::string singer = "voiceDB";
  short track = 1;
  std::string path_lyric = "lyric.txt";
  std::string path_song = "song.wav";
  unsigned long margin = 0;
  bool cache = true;

  // PitchMarker
  unsigned short pitch_margin = 10;

  // BaseWavsMarker
  unsigned char base_wavs_lobe = 3;

  // NoteArranger
  unsigned short ms_front_edge = 30;
  unsigned short ms_back_edge = 30;
  bool sharpen_front = false;
  bool sharpen_back = true;

  // PitchArranger
  unsigned short ms_overshoot = 50;
  double pitch_overshoot = 3.0;
  unsigned short ms_preparation = 50;
  double pitch_preparation = 3.0;
  unsigned short ms_vibrato_offset = 400;
  unsigned short ms_vibrato_width = 200;
  double pitch_vibrato = 3.0;
  bool vibrato = true;
  bool overshoot = true;
  bool preparation = true;
  bool interpolation = true;

  // parser
  void parse(string path_ini)
  {
  	boost::property_tree::ptree ptree;
    try {
  	  read_ini(path_ini, ptree);
    } catch (...) {
      cerr << "[nak::parse] can't find Nakloid.ini" << endl;
      return;
    }

    try {
      string tmp = ptree.get<string>("Nakloid.score_mode");
      if (tmp == "ust")
        score_mode = score_mode_ust;
      else if (tmp == "smf")
        score_mode = score_mode_smf;
      else if (tmp == "nak")
        score_mode = score_mode_nak;
      else
        throw;
    } catch (...) {
      cerr << "[nak::parse] can't recognize score_mode" << endl;
      return;
    }

    switch (score_mode) {
    case score_mode_ust:
      ptree2var<string>(&path_ust, ptree, "Nakloid.path_ust");
      break;
    case score_mode_smf:
      ptree2var<string>(&path_smf, ptree, "Nakloid.path_smf");
      ptree2var<short>(&track, ptree, "Nakloid.track");
      ptree2var<string>(&path_lyric, ptree, "Nakloid.path_lyric");
      break;
    case score_mode_nak:
      break;
    }
    ptree2var<string>(&singer, ptree, "Nakloid.singer");
    ptree2var<string>(&path_song, ptree, "Nakloid.path_song");
    ptree2var<unsigned long>(&margin, ptree, "Nakloid.margin");
    ptree2var<bool>(&cache, ptree, "Nakloid.cache");
    ptree2var<unsigned short>(&pitch_margin, ptree, "PitchMarker.pitch_margin");
    ptree2var<unsigned short>(&ms_front_edge, ptree, "NoteArranger.ms_front_edge");
    ptree2var<unsigned short>(&ms_back_edge, ptree, "NoteArranger.ms_back_edge");
    ptree2var<bool>(&sharpen_front, ptree, "NoteArranger.sharpen_front");
    ptree2var<bool>(&sharpen_back, ptree, "NoteArranger.sharpen_back");
    ptree2var<unsigned short>(&ms_overshoot, ptree, "PitchArranger.ms_overshoot");
    ptree2var<double>(&pitch_overshoot, ptree, "PitchArranger.pitch_overshoot");
    ptree2var<unsigned short>(&ms_preparation, ptree, "PitchArranger.ms_preparation");
    ptree2var<double>(&pitch_preparation, ptree, "PitchArranger.pitch_preparation");
    ptree2var<unsigned short>(&ms_vibrato_offset, ptree, "PitchArranger.ms_vibrato_offset");
    ptree2var<unsigned short>(&ms_vibrato_width, ptree, "PitchArranger.ms_vibrato_width");
    ptree2var<double>(&pitch_vibrato, ptree, "PitchArranger.pitch_vibrato");
    ptree2var<bool>(&vibrato, ptree, "PitchArranger.vibrato");
    ptree2var<bool>(&overshoot, ptree, "PitchArranger.overshoot");
    ptree2var<bool>(&preparation, ptree, "PitchArranger.preparation");
    ptree2var<bool>(&interpolation, ptree, "PitchArranger.interpolation");
  }

  template <class T> void ptree2var(T *var, boost::property_tree::ptree ptree, string path)
  {
      cout << path << endl;
    try {
      *var = ptree.get<T>(path);
    } catch (...) {
      cerr << "[nak::ptree2var] can't find "+path << endl;
    }
    return;
  }
}
