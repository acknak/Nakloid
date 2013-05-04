#include "Utilities.h"

using namespace std;
using namespace nak;

namespace nak {
  // General
  enum ScoreMode score_mode;
  enum PitchesMode pitches_mode;
  string path_pitches;
  string path_singer;
  string path_song;
  unsigned long margin;
  unsigned char pitch_frame_length;

  // General nak
  string path_nak;

  // General ust
  string path_ust;

  // General smf
  short track;
  string path_smf;
  string path_lyrics;

  // Nakloid
  bool cache;
  string path_log;
  bool vowel_combining;
  double vowel_combining_volume;
  string path_output_nak;
  string path_output_pit;
  string path_prefix_map;

  // PitchMarker
  unsigned short pitch_margin;

  // BaseWavsMaker
  short target_rms;
  unsigned char base_wavs_lobe;
  bool is_normalize;

  // BaseWavsOverlapper
  double fade_stretch;
  bool compressor;
  double threshold_x;
  double threshold_y;
  double max_volume;

  // NoteArranger
  unsigned short ms_front_edge;
  unsigned short ms_back_edge;

  // PitchArranger
  unsigned short ms_overshoot;
  double pitch_overshoot;
  unsigned short ms_preparation;
  double pitch_preparation;
  unsigned short ms_vibrato_offset;
  unsigned short ms_vibrato_width;
  double pitch_vibrato;
  double finefluctuation_deviation;
  bool vibrato;
  bool overshoot;
  bool preparation;
  bool interpolation;
  bool finefluctuation;

  // internal parameters
  map<string, string> vow2pron = boost::assign::map_list_of
    ("a", "‚ ")("i", "‚¢")("u", "‚¤")("e", "‚¦")("o", "‚¨")("n", "‚ñ")("i", "‚î")("e", "‚ï");
  map<string, string> pron2vow = boost::assign::map_list_of
    ("‚ ", "a")("‚¢", "i")("‚¤", "u")("‚¦", "e")("‚¨", "o")
    ("‚Ÿ", "a")("‚¡", "i")("‚£", "u")("‚¥", "e")("‚§", "o")
    ("‚©", "a")("‚«", "i")("‚­", "u")("‚¯", "e")("‚±", "o")
    ("‚ª", "a")("‚¬", "i")("‚®", "u")("‚°", "e")("‚²", "o")
    ("‚³", "a")("‚µ", "i")("‚·", "u")("‚¹", "e")("‚»", "o")
    ("‚´", "a")("‚¶", "i")("‚¸", "u")("‚º", "e")("‚¼", "o")
    ("‚½", "a")("‚¿", "i")("‚Â", "u")("‚Ä", "e")("‚Æ", "o")
    ("‚¾", "a")("‚À", "i")("‚Ã", "u")("‚Å", "e")("‚Ç", "o")
    ("‚È", "a")("‚É", "i")("‚Ê", "u")("‚Ë", "e")("‚Ì", "o")
    ("‚Í", "a")("‚Ð", "i")("‚Ó", "u")("‚Ö", "e")("‚Ù", "o")
    ("‚Î", "a")("‚Ñ", "i")("‚Ô", "u")("‚×", "e")("‚Ú", "o")
    ("‚Ï", "a")("‚Ò", "i")("‚Õ", "u")("‚Ø", "e")("‚Û", "o")
    ("‚Ü", "a")("‚Ý", "i")("‚Þ", "u")("‚ß", "e")("‚à", "o")
    ("‚â", "a")("‚ä", "u")("‚æ", "o")("‚á", "a")("‚ã", "u")("‚å", "o")
    ("‚ç", "a")("‚è", "i")("‚é", "u")("‚ê", "e")("‚ë", "o")
    ("‚í", "a")("‚ð", "o")("‚ñ", "n")("‚î", "i")("‚ï", "e");
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
    path_nak = ptree.get<string>("General.path_nak", "score.nak");
  } else if (tmp_score == "ust") {
    score_mode = score_mode_ust;
    path_ust = ptree.get<string>("General.path_ust", "score.ust");
  } else if (tmp_score == "smf") {
    score_mode = score_mode_smf;
    track = ptree.get<short>("General.track", 1);
    path_smf = ptree.get<string>("General.path_smf", "score.mid");
    path_lyrics = ptree.get<string>("General.path_lyrics", "lyrics.txt");
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
  path_pitches = ptree.get<string>("General.path_pitches", "");
  path_singer = ptree.get<string>("General.path_singer", "");
  path_song = ptree.get<string>("General.path_song", "");
  margin = ptree.get<unsigned long>("General.margin", 0);
  pitch_frame_length = ptree.get<unsigned char>("General.pitch_frame_length", 1);

  // Nakloid
  cache = ptree.get<bool>("Nakloid.cache", false);
  path_log = ptree.get<string>("Nakloid.path_log", "");
  vowel_combining = ptree.get<bool>("Nakloid.vowel_combining", false);
  vowel_combining_volume = ptree.get<double>("Nakloid.vowel_combining_volume", 1.0);
  path_output_nak = ptree.get<string>("Nakloid.path_output_nak", "");
  path_output_pit = ptree.get<string>("Nakloid.path_output_pit", "");
  path_prefix_map = ptree.get<string>("Nakloid.path_prefix_map", "");

  // PitchMarker
  pitch_margin = ptree.get<unsigned short>("PitchMarker.pitch_margin", 10);

  // BaseWavsMaker
  target_rms = ptree.get<short>("BaseWavsMaker.target_rms", 2400);
  base_wavs_lobe = ptree.get<unsigned char>("BaseWavsMaker.base_wavs_lobe", 3);
  is_normalize = ptree.get<bool>("BaseWavsMaker.normalize", false);

  // BaseWavsOverlapper
  fade_stretch = ptree.get<double>("BaseWavsOverlapper.fade_stretch", 1.0);
  compressor = ptree.get<bool>("BaseWavsOverlapper.compressor", false);
  threshold_x = ptree.get<double>("BaseWavsOverlapper.threshold_x", 0.98);
  threshold_y = ptree.get<double>("BaseWavsOverlapper.threshold_y", 0.98);
  max_volume = ptree.get<double>("BaseWavsOverlapper.max_volume", 0.9);

  // NoteArranger
  ms_front_edge = ptree.get<unsigned short>("NoteArranger.ms_front_edge", 5);
  ms_back_edge = ptree.get<unsigned short>("NoteArranger.ms_back_edge", 35);

  // PitchArranger
  ms_overshoot = ptree.get<unsigned short>("PitchArranger.ms_overshoot", 50);
  pitch_overshoot = ptree.get<double>("PitchArranger.pitch_overshoot", 3.0);
  ms_preparation = ptree.get<unsigned short>("PitchArranger.ms_preparation", 50);
  pitch_preparation = ptree.get<double>("PitchArranger.pitch_preparation", 3.0);
  ms_vibrato_offset = ptree.get<unsigned short>("PitchArranger.ms_vibrato_offset", 400);
  ms_vibrato_width = ptree.get<unsigned short>("PitchArranger.ms_vibrato_width", 200);
  pitch_vibrato = ptree.get<double>("PitchArranger.pitch_vibrato", 3.0);
  finefluctuation_deviation = ptree.get<double>("PitchArranger.finefluctuation_deviation", 0.5);
  vibrato = ptree.get<bool>("PitchArranger.vibrato", false);
  overshoot = ptree.get<bool>("PitchArranger.overshoot", false);
  preparation = ptree.get<bool>("PitchArranger.preparation", false);
  interpolation = ptree.get<bool>("PitchArranger.interpolation", false);
  finefluctuation = ptree.get<bool>("PitchArranger.finefluctuation", false);

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

vector<short> nak::normalize(vector<short> wav, double target_rms)
{
  double scale = target_rms / getRMS(wav);
  for (int i=0; i<wav.size(); i++)
    wav[i] *= scale;
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

double nak::getDB(long wav_value)
{
  if (wav_value == 0)
    return 1;
  double tmp = log10(abs(wav_value)/32768.0) * 20;
  return (tmp>0)?1:tmp;
}

short nak::reverseDB(double db)
{
  if (db > 0)
    return 32767;
  return (db>0)?32726:pow(10,db/20)*32767;
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

map<string, string>::const_iterator nak::getVow2PronIt(string pron)
{
  map<string, string>::const_iterator it = nak::vow2pron.begin();
  do {
    if (it->second == pron) {
      break;
    }
  } while (++it != nak::vow2pron.end());
  return it;
}
