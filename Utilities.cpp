#include "Utilities.h"

using namespace std;
using namespace nak;

namespace nak {
  // Input
  wstring path_singer;
  wstring path_prefix_map;

  enum ScoreMode score_mode;
  wstring path_nak;
  wstring path_ust;
  wstring path_smf;
  short track;
  wstring path_lyrics;

  enum PitchesMode pitches_mode;
  wstring path_pitches;
  unsigned char pitch_frame_length;

  // Output
  wstring path_song;
  wstring path_output_nak;
  wstring path_output_pit;

  // Nakloid
  long margin;
  bool cache;
  bool print_log;
  bool auto_vowel_combining;
  double vowel_combining_volume;

  // PitchMarker
  short pitch_margin;
  double xcorr_threshold;

  // UnitWaveformMaker
  double target_rms;
  unsigned char unit_waveform_lobe;
  bool uwc_normalize;
  double max_scale_ratio;

  // UnitWaveformOverlapper
  double fade_stretch;
  bool interpolation;
  bool wav_normalize;
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
  bool completion;
  bool finefluctuation;

  // internal parameters
  map<wstring, wstring> vow2pron = boost::assign::map_list_of
    (L"a", L"あ")(L"i", L"い")(L"u", L"う")(L"e", L"え")(L"o", L"お")(L"n", L"ん");
  map<wstring, wstring> pron2vow = boost::assign::map_list_of
    (L"あ", L"a")(L"い", L"i")(L"う", L"u")(L"え", L"e")(L"お", L"o")
    (L"ぁ", L"a")(L"ぃ", L"i")(L"ぅ", L"u")(L"ぇ", L"e")(L"ぉ", L"o")
    (L"か", L"a")(L"き", L"i")(L"く", L"u")(L"け", L"e")(L"こ", L"o")
    (L"が", L"a")(L"ぎ", L"i")(L"ぐ", L"u")(L"げ", L"e")(L"ご", L"o")
    (L"さ", L"a")(L"し", L"i")(L"す", L"u")(L"せ", L"e")(L"そ", L"o")
    (L"ざ", L"a")(L"じ", L"i")(L"ず", L"u")(L"ぜ", L"e")(L"ぞ", L"o")
    (L"た", L"a")(L"ち", L"i")(L"つ", L"u")(L"て", L"e")(L"と", L"o")
    (L"だ", L"a")(L"ぢ", L"i")(L"づ", L"u")(L"で", L"e")(L"ど", L"o")
    (L"な", L"a")(L"に", L"i")(L"ぬ", L"u")(L"ね", L"e")(L"の", L"o")
    (L"は", L"a")(L"ひ", L"i")(L"ふ", L"u")(L"へ", L"e")(L"ほ", L"o")
    (L"ば", L"a")(L"び", L"i")(L"ぶ", L"u")(L"べ", L"e")(L"ぼ", L"o")
    (L"ぱ", L"a")(L"ぴ", L"i")(L"ぷ", L"u")(L"ぺ", L"e")(L"ぽ", L"o")
    (L"ま", L"a")(L"み", L"i")(L"む", L"u")(L"め", L"e")(L"も", L"o")
    (L"や", L"a")(L"ゆ", L"u")(L"よ", L"o")(L"ゃ", L"a")(L"ゅ", L"u")(L"ょ", L"o")
    (L"ら", L"a")(L"り", L"i")(L"る", L"u")(L"れ", L"e")(L"ろ", L"o")
    (L"わ", L"a")(L"を", L"o")(L"ん", L"n")(L"ゐ", L"i")(L"ゑ", L"e");
}

// parser
bool nak::parse(const wstring& path_ini)
{
  boost::property_tree::wptree ptree;
  try {
    boost::filesystem::path fs_path_ini(path_ini);
    boost::property_tree::ini_parser::read_ini(fs_path_ini.string(), ptree);
  } catch (boost::property_tree::ini_parser::ini_parser_error &e) {
    cerr << "[Nakloid.ini line " << e.line() << "] " << e.message() << endl
      << "[nak::parse] can't parse Nakloid.ini" << endl;
    return false;
  } catch (...) {
    cerr << "[nak::parse] can't parse Nakloid.ini" << endl;
  }

  // Input
  path_singer = ptree.get<wstring>(L"Input.path_singer", L"");
  path_prefix_map = ptree.get<wstring>(L"Input.path_prefix_map", L"");

  {
    wstring tmp_score = ptree.get<wstring>(L"Input.score_mode", L"nak");
    if (tmp_score == L"nak") {
      score_mode = score_mode_nak;
      path_nak = ptree.get<wstring>(L"Input.path_nak", L"score.nak");
    } else if (tmp_score == L"ust") {
      score_mode = score_mode_ust;
      path_ust = ptree.get<wstring>(L"Input.path_ust", L"score.ust");
    } else if (tmp_score == L"smf") {
      score_mode = score_mode_smf;
      track = ptree.get<short>(L"Input.track", 1);
      path_smf = ptree.get<wstring>(L"Input.path_smf", L"score.mid");
      path_lyrics = ptree.get<wstring>(L"Input.path_lyrics", L"lyrics.txt");
    } else {
      cerr << "[nak::parse] can't recognize score_mode" << endl;
      return false;
    }
  }
  {
    wstring tmp_pitches = ptree.get<wstring>(L"Input.pitches_mode", L"");
    if (tmp_pitches == L"pit") {
      pitches_mode = pitches_mode_pit;
    } else if (tmp_pitches == L"lf0") {
      pitches_mode = pitches_mode_lf0;
    } else {
      pitches_mode = pitches_mode_none;
    }
  }
  path_pitches = ptree.get<wstring>(L"Input.path_pitches", L"");
  pitch_frame_length = ptree.get<unsigned char>(L"Input.pitch_frame_length", 1);

  // Output
  margin = ptree.get<long>(L"Output.margin", 0);
  path_song = ptree.get<wstring>(L"Output.path_song", L"");
  path_output_nak = ptree.get<wstring>(L"Output.path_output_nak", L"");
  path_output_pit = ptree.get<wstring>(L"Output.path_output_pit", L"");

  // Nakloid
  cache = ptree.get<bool>(L"Nakloid.cache", false);
  print_log = ptree.get<bool>(L"Nakloid.print_log", "");
  auto_vowel_combining = ptree.get<bool>(L"Nakloid.auto_vowel_combining", false);
  vowel_combining_volume = ptree.get<double>(L"Nakloid.vowel_combining_volume", 1.0);

  // PitchMarker
  pitch_margin = ptree.get<short>(L"PitchMarker.pitch_margin", 10);
  xcorr_threshold = ptree.get<double>(L"PitchMarker.xcorr_threshold", 0.95);

  // UnitWaveformMaker
  target_rms = ptree.get<double>(L"UnitWaveformMaker.target_rms", 0.05);
  unit_waveform_lobe = ptree.get<unsigned char>(L"UnitWaveformMaker.unit_waveform_lobe", 3);
  uwc_normalize = ptree.get<bool>(L"UnitWaveformMaker.uwc_normalize", false);
  max_scale_ratio = ptree.get<double>(L"UnitWaveformMaker.max_scale_ratio", 1.0);

  // UnitWaveformOverlapper
  fade_stretch = ptree.get<double>(L"UnitWaveformOverlapper.fade_stretch", 1.0);
  interpolation = ptree.get<bool>(L"UnitWaveformOverlapper.interpolation", false);
  wav_normalize = ptree.get<bool>(L"UnitWaveformOverlapper.wav_normalize", false);
  max_volume = ptree.get<double>(L"UnitWaveformOverlapper.max_volume", 0.9);
  compressor = ptree.get<bool>(L"UnitWaveformOverlapper.compressor", false);
  threshold = ptree.get<double>(L"UnitWaveformOverlapper.threshold", -18.0);
  ratio = ptree.get<double>(L"UnitWaveformOverlapper.threshold_y", 2.5);

  // Note
  ms_front_padding = ptree.get<short>(L"Note.ms_front_padding", 5);
  ms_back_padding = ptree.get<short>(L"Note.ms_back_padding", 35);

  // PitchArranger
  vibrato = ptree.get<bool>(L"PitchArranger.vibrato", false);
  overshoot = ptree.get<bool>(L"PitchArranger.overshoot", false);
  preparation = ptree.get<bool>(L"PitchArranger.preparation", false);
  completion = ptree.get<bool>(L"PitchArranger.completion", true);
  finefluctuation = ptree.get<bool>(L"PitchArranger.finefluctuation", false);
  ms_overshoot = ptree.get<short>(L"PitchArranger.ms_overshoot", 50);
  pitch_overshoot = ptree.get<double>(L"PitchArranger.pitch_overshoot", 3.0);
  ms_preparation = ptree.get<short>(L"PitchArranger.ms_preparation", 50);
  pitch_preparation = ptree.get<double>(L"PitchArranger.pitch_preparation", 3.0);
  ms_vibrato_offset = ptree.get<short>(L"PitchArranger.ms_vibrato_offset", 400);
  ms_vibrato_width = ptree.get<short>(L"PitchArranger.ms_vibrato_width", 200);
  pitch_vibrato = ptree.get<double>(L"PitchArranger.pitch_vibrato", 3.0);
  finefluctuation_deviation = ptree.get<double>(L"PitchArranger.finefluctuation_deviation", 0.5);

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

double nak::dB2val(const pair<bool, double>& dB)
{
  if (dB.second > 0)
    return 0.0;
  return pow(10, dB.second/20)*(dB.first?1:-1);
}

double nak::cent2rate(const double cent)
{
  return pow(2, cent/1200.0);
}

vector<double> nak::normalize(const vector<double>& wav, double target_rms)
{
  vector<double> tmp_wav = wav;
  double scale = target_rms / getRMS(tmp_wav);
  for (size_t i=0; i<tmp_wav.size(); i++)
    tmp_wav[i] *= scale;
  return tmp_wav;
}

vector<double> nak::normalize(const vector<double>& wav, double target_mean, double target_var)
{
  vector<double> tmp_wav = wav;
  double wav_mean = getMean(tmp_wav);
  double wav_var = getVar(tmp_wav, wav_mean);
  for (size_t i=0; i<tmp_wav.size(); i++)
    tmp_wav[i] = (tmp_wav[i]+(target_mean-wav_mean)) * (target_var/wav_var);
  return tmp_wav;
}

vector<double> nak::normalize(const vector<double>& wav, short target_max, short target_min)
{
  vector<double> tmp_wav = wav;
  double wav_max = *max_element(tmp_wav.begin(), tmp_wav.end());
  double wav_min = *min_element(tmp_wav.begin(), tmp_wav.end());

  for (size_t i=0; i<tmp_wav.size(); i++)
    tmp_wav[i] -= ((wav_max+wav_min)/2.0);

  for (size_t i=0; i<tmp_wav.size(); i++)
    tmp_wav[i] *= ((double)target_max-target_min) / (wav_max-wav_min);

  for (size_t i=0; i<tmp_wav.size(); i++)
    tmp_wav[i] += (target_max+target_min)/2.0;

  return tmp_wav;
}

double nak::getRMS(const vector<double>& wav)
{
  double rms = 0.0;
  for (size_t i=0; i<wav.size(); i++)
    rms += pow((double)wav[i], 2) / wav.size();
  return sqrt(rms);
}

double nak::getMean(const vector<double>& wav)
{
  double mean = 0.0;
  for (size_t i=0; i<wav.size(); i++)
    mean += wav[i] / (double)wav.size();
  return mean;
}

double nak::getVar(const vector<double>& wav, double mean)
{
  double var = 0.0;
  for (size_t i=0; i<wav.size(); i++)
    var += pow(wav[i]-mean, 2) / wav.size();
  return sqrt(var);
}

vector<double> nak::getTri(long len)
{
  vector<double> filter(len, 0);
  for (size_t i=0; i<filter.size(); ++i) {
    double x = (i+1.0) / (filter.size()+1.0);
    filter[i] = 1.0 - 2*fabs(x-0.5);
  }
  return filter;
}

vector<double> nak::getHann(long len)
{
  vector<double> filter(len, 0);
  for (size_t i=0; i<filter.size(); ++i) {
    double x = (i+1.0) / (filter.size()+1.0);
    filter[i] = 0.5 - (0.5 * cos(2*M_PI*x));
  }
  return filter;
}

vector<double> nak::getLanczos(long len, unsigned char lobe)
{
  vector<double> fore_filter(len/2, 0);
  vector<double> aft_filter(len/2, 0);

  for (size_t i=0; i<fore_filter.size(); i++) {
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

map<wstring, wstring>::const_iterator nak::getVow2PronIt(const wstring& pron)
{
  map<wstring, wstring>::const_iterator it = nak::vow2pron.begin();
  do {
    if (it->second == pron) {
      break;
    }
  } while (++it != nak::vow2pron.end());
  return it;
}

nak::VoiceAlias::VoiceAlias(const wstring &alias)
{
  pron = alias;
  bool is_vcv = false;

  // get prefix
  wstring::size_type pos_prefix = pron.find(L" ");
  if (pron.size()>1 && pos_prefix != wstring::npos) {
    prefix = pron.substr(0, pos_prefix+1);
    pron.erase(0, pos_prefix+1);
  }

  // get suffix
  if (pron.size() > 2) {
    wstring tmp_suffix = pron.substr(pron.size()-2);
    if (iswascii(tmp_suffix[0]) && iswalpha(tmp_suffix[0]) && iswdigit(tmp_suffix[1])) {
      suffix = tmp_suffix;
      pron.erase(pron.size()-2, 2);
    }
  }
}

bool nak::VoiceAlias::checkVCV()
{
  return (prefix.find(L" ")!=wstring::npos && (prefix[0]!=L'*'&&prefix[0]!=L'-'));
}
