#include "Utilities.h"

using namespace std;
using namespace nak;

namespace nak {
  // Input
  string path_singer;
  string path_prefix_map;

  enum ScoreMode score_mode;
  string path_nak;
  string path_ust;
  string path_smf;
  short track;
  string path_lyrics;

  enum PitchesMode pitches_mode;
  string path_pitches;
  unsigned char pitch_frame_length;

  // Output
  string path_song;
  string path_output_nak;
  string path_output_pit;

  // Nakloid
  long margin;
  bool cache;
  string path_log;
  bool auto_vowel_combining;
  double vowel_combining_volume;

  // PitchMarker
  short pitch_margin;

  // UnitWaveformMaker
  double target_rms;
  unsigned char unit_waveform_lobe;
  bool is_normalize;

  // UnitWaveformOverlapper
  double fade_stretch;
  double max_volume;
  bool compressor;
  double threshold;
  double ratio;

  // Note
  short ms_front_padding;
  short ms_back_padding;

  // PitchArranger
  short ms_overshoot;
  double pitch_overshoot;
  short ms_preparation;
  double pitch_preparation;
  short ms_vibrato_offset;
  short ms_vibrato_width;
  double pitch_vibrato;
  double finefluctuation_deviation;
  bool vibrato;
  bool overshoot;
  bool preparation;
  bool interpolation;
  bool finefluctuation;

  // internal parameters
  map<string, string> vow2pron = boost::assign::map_list_of
    ("a", "あ")("i", "い")("u", "う")("e", "え")("o", "お")("n", "ん")("i", "ゐ")("e", "ゑ");
  map<string, string> pron2vow = boost::assign::map_list_of
    ("あ", "a")("い", "i")("う", "u")("え", "e")("お", "o")
    ("ぁ", "a")("ぃ", "i")("ぅ", "u")("ぇ", "e")("ぉ", "o")
    ("か", "a")("き", "i")("く", "u")("け", "e")("こ", "o")
    ("が", "a")("ぎ", "i")("ぐ", "u")("げ", "e")("ご", "o")
    ("さ", "a")("し", "i")("す", "u")("せ", "e")("そ", "o")
    ("ざ", "a")("じ", "i")("ず", "u")("ぜ", "e")("ぞ", "o")
    ("た", "a")("ち", "i")("つ", "u")("て", "e")("と", "o")
    ("だ", "a")("ぢ", "i")("づ", "u")("で", "e")("ど", "o")
    ("な", "a")("に", "i")("ぬ", "u")("ね", "e")("の", "o")
    ("は", "a")("ひ", "i")("ふ", "u")("へ", "e")("ほ", "o")
    ("ば", "a")("び", "i")("ぶ", "u")("べ", "e")("ぼ", "o")
    ("ぱ", "a")("ぴ", "i")("ぷ", "u")("ぺ", "e")("ぽ", "o")
    ("ま", "a")("み", "i")("む", "u")("め", "e")("も", "o")
    ("や", "a")("ゆ", "u")("よ", "o")("ゃ", "a")("ゅ", "u")("ょ", "o")
    ("ら", "a")("り", "i")("る", "u")("れ", "e")("ろ", "o")
    ("わ", "a")("を", "o")("ん", "n")("ゐ", "i")("ゑ", "e");
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

  // Input
  path_singer = ptree.get<string>("Input.path_singer", "");
  path_prefix_map = ptree.get<string>("Input.path_prefix_map", "");

  string tmp_score = ptree.get<string>("Input.score_mode", "nak");
  if (tmp_score == "nak") {
    score_mode = score_mode_nak;
    path_nak = ptree.get<string>("Input.path_nak", "score.nak");
  } else if (tmp_score == "ust") {
    score_mode = score_mode_ust;
    path_ust = ptree.get<string>("Input.path_ust", "score.ust");
  } else if (tmp_score == "smf") {
    score_mode = score_mode_smf;
    track = ptree.get<short>("Input.track", 1);
    path_smf = ptree.get<string>("Input.path_smf", "score.mid");
    path_lyrics = ptree.get<string>("Input.path_lyrics", "lyrics.txt");
  } else {
    cerr << "[nak::parse] can't recognize score_mode" << endl;
    return false;
  }
  string tmp_pitches = ptree.get<string>("Input.pitches_mode", "");
  if (tmp_pitches == "pit") {
    pitches_mode = pitches_mode_pit;
  } else if (tmp_pitches == "lf0") {
    pitches_mode = pitches_mode_lf0;
  } else {
    pitches_mode = pitches_mode_none;
  }
  path_pitches = ptree.get<string>("Input.path_pitches", "");
  pitch_frame_length = ptree.get<unsigned char>("Input.pitch_frame_length", 1);

  // Output
  margin = ptree.get<long>("Output.margin", 0);
  path_song = ptree.get<string>("Output.path_song", "");
  path_output_nak = ptree.get<string>("Output.path_output_nak", "");
  path_output_pit = ptree.get<string>("Output.path_output_pit", "");

  // Nakloid
  cache = ptree.get<bool>("Nakloid.cache", false);
  path_log = ptree.get<string>("Nakloid.path_log", "");
  auto_vowel_combining = ptree.get<bool>("Nakloid.auto_vowel_combining", false);
  vowel_combining_volume = ptree.get<double>("Nakloid.vowel_combining_volume", 1.0);

  // PitchMarker
  pitch_margin = ptree.get<short>("PitchMarker.pitch_margin", 10);

  // UnitWaveformMaker
  target_rms = ptree.get<double>("UnitWaveformMaker.target_rms", 0.05);
  unit_waveform_lobe = ptree.get<unsigned char>("UnitWaveformMaker.unit_waveform_lobe", 3);
  is_normalize = ptree.get<bool>("UnitWaveformMaker.normalize", false);

  // UnitWaveformOverlapper
  fade_stretch = ptree.get<double>("UnitWaveformOverlapper.fade_stretch", 1.0);
  max_volume = ptree.get<double>("UnitWaveformOverlapper.max_volume", 0.9);
  compressor = ptree.get<bool>("UnitWaveformOverlapper.compressor", false);
  threshold = ptree.get<double>("UnitWaveformOverlapper.threshold", -18.0);
  ratio = ptree.get<double>("UnitWaveformOverlapper.threshold_y", 2.5);

  // Note
  ms_front_padding = ptree.get<short>("Note.ms_front_padding", 5);
  ms_back_padding = ptree.get<short>("Note.ms_back_padding", 35);

  // PitchArranger
  vibrato = ptree.get<bool>("PitchArranger.vibrato", false);
  overshoot = ptree.get<bool>("PitchArranger.overshoot", false);
  preparation = ptree.get<bool>("PitchArranger.preparation", false);
  interpolation = ptree.get<bool>("PitchArranger.interpolation", false);
  finefluctuation = ptree.get<bool>("PitchArranger.finefluctuation", false);
  ms_overshoot = ptree.get<short>("PitchArranger.ms_overshoot", 50);
  pitch_overshoot = ptree.get<double>("PitchArranger.pitch_overshoot", 3.0);
  ms_preparation = ptree.get<short>("PitchArranger.ms_preparation", 50);
  pitch_preparation = ptree.get<double>("PitchArranger.pitch_preparation", 3.0);
  ms_vibrato_offset = ptree.get<short>("PitchArranger.ms_vibrato_offset", 400);
  ms_vibrato_width = ptree.get<short>("PitchArranger.ms_vibrato_width", 200);
  pitch_vibrato = ptree.get<double>("PitchArranger.pitch_vibrato", 3.0);
  finefluctuation_deviation = ptree.get<double>("PitchArranger.finefluctuation_deviation", 0.5);

  return true;
}

long nak::ms2pos(long ms, WavFormat format)
{
  return (long)(ms/1000.0*format.dwSamplesPerSec);
}

long nak::pos2ms(long pos, WavFormat format)
{
  return (long)(pos/(double)format.dwSamplesPerSec*1000);
}

long nak::tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo)
{
  return (long)(((double)tick) / timebase * (tempo/1000.0));
}

vector<double> nak::normalize(vector<double> wav, double target_rms)
{
  double scale = target_rms / getRMS(wav);
  for (int i=0; i<wav.size(); i++)
    wav[i] *= scale;
  return wav;
}

vector<double> nak::normalize(vector<double> wav, double target_mean, double target_var)
{
  double wav_mean = getMean(wav);
  double wav_var = getVar(wav, wav_mean);
  for (int i=0; i<wav.size(); i++)
    wav[i] = (wav[i]+(target_mean-wav_mean)) * (target_var/wav_var);
  return wav;
}

vector<double> nak::normalize(vector<double> wav, short target_max, short target_min)
{
  double wav_max = *max_element(wav.begin(), wav.end());
  double wav_min = *min_element(wav.begin(), wav.end());

  for (int i=0; i<wav.size(); i++)
    wav[i] -= ((wav_max+wav_min)/2.0);

  for (int i=0; i<wav.size(); i++)
    wav[i] *= ((double)target_max-target_min) / (wav_max-wav_min);

  for (int i=0; i<wav.size(); i++)
    wav[i] += (target_max+target_min)/2.0;

  return wav;
}

double nak::getRMS(vector<double> wav)
{
  double rms = 0.0;
  for (int i=0; i<wav.size(); i++)
    rms += pow((double)wav[i], 2) / wav.size();
  return sqrt(rms);
}

double nak::getMean(vector<double> wav)
{
  double mean = 0.0;
  for (int i=0; i<wav.size(); i++)
    mean += wav[i] / (double)wav.size();
  return mean;
}

double nak::getVar(vector<double> wav, double mean)
{
  double var = 0.0;
  for (int i=0; i<wav.size(); i++)
    var += pow(wav[i]-mean, 2) / wav.size();
  return sqrt(var);
}

pair<bool, double> nak::val2dB(double wav_value)
{
  if (wav_value >= 1.0)
    wav_value = 32768.0/32767.0;
  else if (wav_value<= -1.0)
    wav_value = -32769.0/32768.0;
  else if (wav_value == 0)
    return make_pair(true, 1.0);

  return make_pair(wav_value>0, log10(abs(wav_value))*20);
}

double nak::dB2val(pair<bool, double> dB)
{
  if (dB.second > 0)
    return 0.0;
  return pow(10, dB.second/20)*(dB.first?1:-1);
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

vector<double> nak::getLanczos(long len, unsigned char lobe)
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
