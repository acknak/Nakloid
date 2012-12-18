#include "Utilities.h"

using namespace std;
using namespace nak;

namespace nak {
  // Nakloid
  enum ScoreMode score_mode = score_mode_ust;
  string path_ust = "score.ust";
  string path_smf = "score.mid";
  string singer = "voiceDB";
  short track = 1;
  string path_lyric = "lyric.txt";
  string path_song = "song.wav";
  unsigned long margin = 0;
  bool cache = true;

  // PitchMarker
  unsigned short pitch_margin = 10;

  // BaseWavsMarker
  unsigned char base_wavs_lobe = 3;

  // BaseWavsOverlapper
  bool compressor = true;
  double max_volume = 0.8;
  double threshold_x = 0.6;
  double threshold_y = 0.8;

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
}

// parser
void nak::parse(string path_ini)
{
  boost::property_tree::ptree ptree;
  try {
  	read_ini(path_ini, ptree);
  } catch (boost::property_tree::ini_parser_error &e) {
    cerr << "[nak::parse] can't find Nakloid.ini" << endl;
    return;
  }

  string tmp = ptree.get<string>("Nakloid.score_mode", "ust");
  if (tmp == "ust")
    score_mode = score_mode_ust;
  else if (tmp == "smf")
    score_mode = score_mode_smf;
  else if (tmp == "nak")
    score_mode = score_mode_nak;
  else {
    cerr << "[nak::parse] can't recognize score_mode" << endl;
    return;
  }

  switch (score_mode) {
  case score_mode_ust:
    path_ust = ptree.get<string>("Nakloid.path_ust", path_ust);
    break;
  case score_mode_smf:
    path_smf = ptree.get<string>("Nakloid.path_smf", path_smf);
    track = ptree.get<short>("Nakloid.track", track);
    path_lyric = ptree.get<string>("Nakloid.path_lyric", path_lyric);
    break;
  case score_mode_nak:
    break;
  }
  singer = ptree.get<string>("Nakloid.singer", singer);
  path_song = ptree.get<string>("Nakloid.path_song", path_song);
  margin = ptree.get<unsigned long>("Nakloid.margin", margin);
  cache = ptree.get<bool>("Nakloid.cache", cache);
  pitch_margin = ptree.get<unsigned short>("PitchMarker.pitch_margin", pitch_margin);
  base_wavs_lobe = ptree.get<unsigned char>("BaseWavsMaker.base_wavs_lobe", base_wavs_lobe);
  compressor = ptree.get<bool>("BaseWavsOverlapper.compressor", compressor);
  threshold_x = ptree.get<double>("BaseWavsOverlapper.threshold_x", threshold_x);
  threshold_y = ptree.get<double>("BaseWavsOverlapper.threshold_y", threshold_y);
  max_volume = ptree.get<double>("BaseWavsOverlapper.max_volume", max_volume);
  ms_front_edge = ptree.get<unsigned short>("NoteArranger.ms_front_edge", ms_front_edge);
  ms_back_edge = ptree.get<unsigned short>("NoteArranger.ms_back_edge", ms_back_edge);
  sharpen_front = ptree.get<bool>("NoteArranger.sharpen_front", sharpen_front);
  sharpen_back = ptree.get<bool>("NoteArranger.sharpen_back", sharpen_back);
  ms_overshoot = ptree.get<unsigned short>("PitchArranger.ms_overshoot", ms_overshoot);
  pitch_overshoot = ptree.get<double>("PitchArranger.pitch_overshoot", pitch_overshoot);
  ms_preparation = ptree.get<unsigned short>("PitchArranger.ms_preparation", ms_preparation);
  pitch_preparation = ptree.get<double>("PitchArranger.pitch_preparation", pitch_preparation);
  ms_vibrato_offset = ptree.get<unsigned short>("PitchArranger.ms_vibrato_offset", ms_vibrato_offset);
  ms_vibrato_width = ptree.get<unsigned short>("PitchArranger.ms_vibrato_width", ms_vibrato_width);
  pitch_vibrato = ptree.get<double>("PitchArranger.pitch_vibrato", pitch_vibrato);
  vibrato = ptree.get<bool>("PitchArranger.vibrato", vibrato);
  overshoot = ptree.get<bool>("PitchArranger.overshoot", overshoot);
  preparation = ptree.get<bool>("PitchArranger.preparation", preparation);
  interpolation = ptree.get<bool>("PitchArranger.interpolation", interpolation);
}

unsigned long nak::ms2pos(unsigned long ms, WavFormat format)
{
  return (unsigned long)(ms/1000.0*format.dwSamplesPerSec);
}

unsigned long nak::pos2ms(unsigned long pos, WavFormat format)
{
  return (unsigned long)(pos/(double)format.dwSamplesPerSec*1000);
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
