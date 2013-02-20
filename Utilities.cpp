#include "Utilities.h"

using namespace std;
using namespace nak;

namespace nak {
  // General
  enum ScoreMode score_mode = score_mode_nak;
  enum PitchesMode pitches_mode = pitches_mode_none;
  string path_pitches = "";
  string path_singer = "";
  string path_song = "";
  unsigned long margin = 0;
  unsigned char pitch_frame_length = 1;

  string path_nak = "score.nak";
  string path_ust = "score.ust";
  short track = 1;
  string path_smf = "score.mid";
  string path_lyrics = "lyrics.txt";

  // Nakloid
  bool cache = false;
  string path_log = "";
  bool vowel_combining = false;
  double vowel_combining_volume = 1.0;
  string path_output_nak="";
  string path_output_pit="";

  // PitchMarker
  unsigned short pitch_margin = 10;

  // BaseWavsMarker
  unsigned char base_wavs_lobe = 3;
  bool is_normalize = false;

  // BaseWavsOverlapper
  bool compressor = false;
  double max_volume = 0.8;
  double threshold_x = 0.6;
  double threshold_y = 0.8;

  // NoteArranger
  unsigned short ms_front_edge = 30;
  unsigned short ms_back_edge = 30;
  bool sharpen_front = false;
  bool sharpen_back = false;

  // PitchArranger
  unsigned short ms_overshoot = 50;
  double pitch_overshoot = 3.0;
  unsigned short ms_preparation = 50;
  double pitch_preparation = 3.0;
  unsigned short ms_vibrato_offset = 400;
  unsigned short ms_vibrato_width = 200;
  double pitch_vibrato = 3.0;
  double finefluctuation_deviation = 0.5;
  bool vibrato = false;
  bool overshoot = false;
  bool preparation = false;
  bool interpolation = false;
  bool finefluctuation = false;
}

// parser
bool nak::parse(string path_ini)
{
  boost::property_tree::ptree ptree;
  try {
    boost::property_tree::ini_parser::read_ini(path_ini, ptree);
  } catch (boost::property_tree::ini_parser::ini_parser_error &e) {
    cerr << e.message() << endl
      << "[nak::parse] can't parse Nakloid.ini" << endl;
    return false;
  } catch (...) {
    cerr << "[nak::parse] can't parse Nakloid.ini" << endl;
  }

  // General
  string tmp_score = ptree.get<string>("General.score_mode", "nak");
  if (tmp_score == "nak") {
    score_mode = score_mode_nak;
    path_nak = ptree.get<string>("General.path_nak", path_nak);
  } else if (tmp_score == "ust") {
    score_mode = score_mode_ust;
    path_ust = ptree.get<string>("General.path_ust", path_ust);
  } else if (tmp_score == "smf") {
    score_mode = score_mode_smf;
    track = ptree.get<short>("General.track", track);
    path_smf = ptree.get<string>("General.path_smf", path_smf);
    path_lyrics = ptree.get<string>("General.path_lyrics", path_lyrics);
  } else {
    cerr << "[nak::parse] can't recognize score_mode" << endl;
    return false;
  }
  string tmp_pitches = ptree.get<string>("General.pitches_mode", "");
  if (tmp_pitches == "pit") {
    pitches_mode = pitches_mode_pit;
  } else if (tmp_pitches == "lf0") {
    pitches_mode = pitches_mode_lf0;
  } else {
    pitches_mode = pitches_mode_none;
  }
  path_pitches = ptree.get<string>("General.path_pitches", path_pitches);
  path_singer = ptree.get<string>("General.path_singer", path_singer);
  path_song = ptree.get<string>("General.path_song", path_song);
  margin = ptree.get<unsigned long>("General.margin", margin);
  pitch_frame_length = ptree.get<unsigned char>("General.pitch_frame_length", pitch_frame_length);

  // Nakloid
  cache = ptree.get<bool>("Nakloid.cache", cache);
  path_log = ptree.get<string>("Nakloid.path_log", path_log);
  vowel_combining = ptree.get<bool>("Nakloid.vowel_combining", vowel_combining);
  vowel_combining_volume = ptree.get<double>("Nakloid.vowel_combining_volume", vowel_combining_volume);
  path_output_nak = ptree.get<string>("Nakloid.path_output_nak", path_output_nak);
  path_output_pit = ptree.get<string>("Nakloid.path_output_pit", path_output_pit);

  // PitchMarker
  pitch_margin = ptree.get<unsigned short>("PitchMarker.pitch_margin", pitch_margin);

  // BaseWavsMaker
  base_wavs_lobe = ptree.get<unsigned char>("BaseWavsMaker.base_wavs_lobe", base_wavs_lobe);
  is_normalize = ptree.get<bool>("BaseWavsMaker.normalize", is_normalize);

  // BaseWavsOverlapper
  compressor = ptree.get<bool>("BaseWavsOverlapper.compressor", compressor);
  threshold_x = ptree.get<double>("BaseWavsOverlapper.threshold_x", threshold_x);
  threshold_y = ptree.get<double>("BaseWavsOverlapper.threshold_y", threshold_y);
  max_volume = ptree.get<double>("BaseWavsOverlapper.max_volume", max_volume);

  // NoteArranger
  vowel_combining = ptree.get<bool>("NoteArranger.vowel_combining", vowel_combining);
  ms_front_edge = ptree.get<unsigned short>("NoteArranger.ms_front_edge", ms_front_edge);
  ms_back_edge = ptree.get<unsigned short>("NoteArranger.ms_back_edge", ms_back_edge);
  sharpen_front = ptree.get<bool>("NoteArranger.sharpen_front", sharpen_front);
  sharpen_back = ptree.get<bool>("NoteArranger.sharpen_back", sharpen_back);

  // PitchArranger
  ms_overshoot = ptree.get<unsigned short>("PitchArranger.ms_overshoot", ms_overshoot);
  pitch_overshoot = ptree.get<double>("PitchArranger.pitch_overshoot", pitch_overshoot);
  ms_preparation = ptree.get<unsigned short>("PitchArranger.ms_preparation", ms_preparation);
  pitch_preparation = ptree.get<double>("PitchArranger.pitch_preparation", pitch_preparation);
  ms_vibrato_offset = ptree.get<unsigned short>("PitchArranger.ms_vibrato_offset", ms_vibrato_offset);
  ms_vibrato_width = ptree.get<unsigned short>("PitchArranger.ms_vibrato_width", ms_vibrato_width);
  pitch_vibrato = ptree.get<double>("PitchArranger.pitch_vibrato", pitch_vibrato);
  finefluctuation_deviation = ptree.get<double>("PitchArranger.finefluctuation_deviation", finefluctuation_deviation);
  vibrato = ptree.get<bool>("PitchArranger.vibrato", vibrato);
  overshoot = ptree.get<bool>("PitchArranger.overshoot", overshoot);
  preparation = ptree.get<bool>("PitchArranger.preparation", preparation);
  interpolation = ptree.get<bool>("PitchArranger.interpolation", interpolation);
  finefluctuation = ptree.get<bool>("PitchArranger.finefluctuation", finefluctuation);

  return true;
}

unsigned long nak::ms2pos(unsigned long ms, WavFormat format)
{
  return (unsigned long)(ms/1000.0*format.dwSamplesPerSec);
}

unsigned long nak::pos2ms(unsigned long pos, WavFormat format)
{
  return (unsigned long)(pos/(double)format.dwSamplesPerSec*1000);
}

unsigned long nak::tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo)
{
  return (unsigned long)(((double)tick) / timebase * (tempo/1000.0));
}

double nak::getRMS(vector<short> wav)
{
  double rms = 0.0;
  for (int i=0; i<wav.size(); i++)
    rms += pow((double)wav[i], 2) / wav.size();
  return sqrt(rms);
}

double nak::getMean(vector<short> wav)
{
  double mean = 0.0;
  for (int i=0; i<wav.size(); i++)
    mean += wav[i] / (double)wav.size();
  return mean;
}

double nak::getVar(vector<short> wav, double mean)
{
  double var = 0.0;
  for (int i=0; i<wav.size(); i++)
    var += pow(wav[i]-mean, 2) / wav.size();
  return sqrt(var);
}

vector<short> nak::normalize(vector<short> wav, double target_rms)
{
  double scale = target_rms / getRMS(wav);
  for (int i=0; i<wav.size(); i++)
    wav[i] = wav[i] * scale;
  return wav;
}

vector<short> nak::normalize(vector<short> wav, double target_mean, double target_var)
{
  double wav_mean = getMean(wav);
  double wav_var = getVar(wav, wav_mean);
  for (int i=0; i<wav.size(); i++)
    wav[i] = (wav[i]+(target_mean-wav_mean)) * (target_var/wav_var);
  return wav;
}

vector<short> nak::normalize(vector<short> wav, short target_max, short target_min)
{
  short wav_max = *max_element(wav.begin(), wav.end());
  short wav_min = *min_element(wav.begin(), wav.end());

  for (int i=0; i<wav.size(); i++)
    wav[i] -= ((wav_max+wav_min)/2.0);

  for (int i=0; i<wav.size(); i++)
    wav[i] *= ((double)target_max-target_min) / (wav_max-wav_min);

  for (int i=0; i<wav.size(); i++)
    wav[i] += (target_max+target_min)/2.0;

  return wav;
}

vector<double> nak::getTri(long len)
{
  vector<double> filter(len, 0);
  for (int i=0; i<filter.size(); ++i) {
    double x = (i+1.0) / (filter.size()+1.0);
    filter[i] = 1.0 - 2*fabs(x-0.5);
  }
  return filter;
}

vector<double> nak::getHann(long len)
{
  vector<double> filter(len, 0);
  for (int i=0; i<filter.size(); ++i) {
    double x = (i+1.0) / (filter.size()+1.0);
    filter[i] = 0.5 - (0.5 * cos(2*M_PI*x));
  }
  return filter;
}

vector<double> nak::getLanczos(long len, unsigned short lobe)
{
  vector<double> fore_filter(len/2, 0);
  vector<double> aft_filter(len/2, 0);

  for (int i=0; i<fore_filter.size(); i++) {
    double x = (i+1.0) * lobe / aft_filter.size();
    aft_filter[i] = sinc(x) * sinc(x/lobe);
  }

  reverse_copy(aft_filter.begin(), aft_filter.end(), fore_filter.begin());
  if (len%2 > 0)
    fore_filter.push_back(1.0);

  fore_filter.insert(fore_filter.end(), aft_filter.begin(), aft_filter.end());
  return fore_filter;
}

double nak::sinc(double x)
{
  return sin(M_PI*x)/(M_PI*x);
}

bool nak::isVowel(string pron)
{
  return (pron=="* ‚ "||pron=="* ‚¢"||pron=="* ‚¤"||pron=="* ‚¦"||pron=="* ‚¨"||pron=="* ‚ñ");
}
