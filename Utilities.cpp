#include "Utilities.h"

using namespace std;

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

template <class T> void nak::ptree2var(T *var, boost::property_tree::ptree ptree, string path)
{
    cout << path << endl;
  try {
    *var = ptree.get<T>(path);
  } catch (...) {
    cerr << "[nak::ptree2var] can't find "+path << endl;
  }
  return;
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
