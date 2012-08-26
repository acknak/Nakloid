#include "BaseWavsMaker.h"

using namespace std;

BaseWavsMaker::BaseWavsMaker():lobe(3),rep_start_point(0),rep_len_point(0),rep_start(0){}

BaseWavsMaker::~BaseWavsMaker(){}

vector<BaseWav> BaseWavsMaker::getBaseWavs()
{
  return base_wavs;
}

vector<short> BaseWavsMaker::getVoice()
{
  return voice;
}

void BaseWavsMaker::setVoice(vector<short> voice)
{
  this->voice = voice;
}

vector<long> BaseWavsMaker::getPitchMarkVector()
{
  return pitch_marks;
}

list<long> BaseWavsMaker::getPitchMarkList()
{
  list<long> pitch_marks;

  for (int i=0; this->pitch_marks.size(); i++)
    pitch_marks.push_back(this->pitch_marks[i]);

  return pitch_marks;
}

void BaseWavsMaker::setPitchMarks(vector<long> pitch_marks)
{
  this->pitch_marks = pitch_marks;
}

void BaseWavsMaker::setPitchMarks(list<long> pitch_marks)
{
  this->pitch_marks.reserve(pitch_marks.size());
  for (list<long>::iterator it=pitch_marks.begin(); it!=pitch_marks.end(); ++it)
    this->pitch_marks.push_back(*it);
}

unsigned char BaseWavsMaker::getLobe()
{
  return lobe;
}

void BaseWavsMaker::setLobe(unsigned char lobe)
{
  this->lobe = lobe;
}

void BaseWavsMaker::setRepStart(long rep_start_ms, unsigned long fs)
{
  this->rep_start = rep_start_ms * fs / 1000;
}

long BaseWavsMaker::getRepStartPoint()
{
  return rep_start_point;
}

bool BaseWavsMaker::makeBaseWavs()
{
  if (voice.size()==0 || pitch_marks.empty() || lobe==0)
    return false;
  cout << "----- start making base wavs -----" << endl;

  vector<long> pitches(pitch_marks.size()-1, 0);
  for (int i=0; i<pitches.size(); i++)
    pitches[i] = (pitch_marks[i+1] - pitch_marks[i]);
  pitches.push_back(pitches.back());

  // find repeat point
  if (rep_start > 0) {
    long tmp_rep_start_point = 0;
    for (int i=0; i<pitches.size(); i++) {
      if (rep_start < (tmp_rep_start_point+=pitches[i])) {
        rep_start_point = i;
        rep_len_point = pitches.size() - i - lobe;
        break;
      }
    }
  }
  double mean, std, rep_max, rep_min, rep_tmp;
  rep_tmp = mean = std = 0;
  for (int i=rep_start_point; i<pitches.size(); i++)
    mean += pitches[i];
  mean /= pitches.size();
  for (int i=rep_start_point; i<pitches.size(); i++)
    std += pow(pitches[i]-mean, 2);
  std = sqrt(std / pitches.size());
  rep_max = mean + std;
  rep_min = mean - std;
  for (int i=rep_start_point; i<=pitches.size(); i++) {
    if (i<pitches.size() && pitches[i]<rep_max && pitches[i]>rep_min) {
      if (rep_tmp == 0)
        rep_tmp = i;
    } else {
      if (rep_tmp>0 && rep_len_point<i-rep_tmp) {
        rep_start_point = rep_tmp;
        rep_len_point = i - rep_tmp;
      }
      rep_tmp = 0;
    }
  }
  if (rep_start_point+rep_len_point > pitches.size()-lobe)
    rep_len_point -= (pitches.size()+lobe) - (rep_start_point+rep_len_point);
  cout << "pitch mean:" << mean << ", pitch std:" << std << endl;

  if (rep_len_point%2 > 0) {
    ++rep_start_point;
    --rep_len_point;
  }
  cout << "pitches size:" << pitches.size() << ", rep_start_point:" << rep_start_point << ", rep_len_point:" << rep_len_point << endl;

  // make base wavs
  base_wavs.clear();
  base_wavs.reserve(rep_start_point+rep_len_point);
  for (int i=0; i<rep_start_point+rep_len_point; i++) {
    // set lobe size
    unsigned short tmp_lobe = 0;
    while (tmp_lobe<lobe
      && pitch_marks[i]-(pitches[i]*tmp_lobe) >= 0
      && pitch_marks[i]+(pitches[i]*tmp_lobe) < voice.size())
      ++tmp_lobe;
    // cut window out
    BaseWav base_wav;
    long win_start = pitch_marks[i] - (pitches[i]*tmp_lobe);
    long win_end = pitch_marks[i] + (pitches[i]*tmp_lobe);
    vector<double> filter = getLanczos(pitches[i]*tmp_lobe*2+1 ,tmp_lobe);
    base_wav.fact.dwPitchLeft = pitches[i] * tmp_lobe;
    base_wav.fact.dwPitchRight = pitches[i] * tmp_lobe;
    base_wav.fact.dwPosition = pitch_marks[i];

    if (win_start < 0) {
      // left edge
      filter.erase(filter.begin(), filter.begin()-win_start);
      base_wav.fact.dwPitchLeft += win_start;
      win_start = 0;
    }
    if (win_end >= voice.size()) {
      // right edge
      filter.erase((filter.end()-1)-(win_end-voice.size()), filter.end());
      base_wav.fact.dwPitchRight = win_end - voice.size();
      win_end = voice.size() - 1;
    }
    // set base wav data
    vector<short> base_wav_data(win_end-win_start+1, 0);
    for (int j=0; j<base_wav_data.size(); j++)
      base_wav_data[j] = voice[win_start+j] * filter[j];
    base_wav.data.setData(base_wav_data);
    base_wavs.push_back(base_wav);
  }

  // make repeat morphing
  vector<double> filter = getTri(rep_len_point);
  filter.erase(filter.begin()+(filter.size()/2), filter.end());
  long base_pos = rep_start_point + (rep_len_point/2);
  double target_rms = getRMS(base_wavs[rep_start_point].data.getDataVector());
  cout << "base_wavs size:" << base_wavs.size() << ", base_pos:" << base_pos << endl;
  for (int i=0; i<rep_len_point/2; i++) {
    BaseWav fore_wav = base_wavs[rep_start_point+i];
    BaseWav aft_wav = base_wavs[base_pos+i];
    vector<short> fore_wav_data = fore_wav.data.getDataVector();
    vector<short> aft_wav_data = aft_wav.data.getDataVector();
    vector<short> morph_wav_data(aft_wav_data.size(), 0);

    // left
    long left_diff = aft_wav.fact.dwPitchLeft - fore_wav.fact.dwPitchLeft;
    if (left_diff < 0) {
      for (int j=0; j<aft_wav.fact.dwPitchLeft; j++) {
        morph_wav_data[j] = fore_wav_data[-left_diff+j]*filter[i] + aft_wav_data[j]*(1-filter[i]);
      }
    } else {
      vector<short> tmp_fore_wav_data(fore_wav_data);
      tmp_fore_wav_data.insert(tmp_fore_wav_data.begin(), left_diff, 0);
      for (int j=0; j<aft_wav.fact.dwPitchLeft; j++)
        morph_wav_data[j] = tmp_fore_wav_data[j]*filter[i] + aft_wav_data[j]*(1-filter[i]);
    }

    // center
    morph_wav_data[aft_wav.fact.dwPitchLeft] =
      fore_wav_data[fore_wav.fact.dwPitchLeft]*filter[i]
      + aft_wav_data[aft_wav.fact.dwPitchLeft]*(1-filter[i]);

    // right
    long right_diff = aft_wav.fact.dwPitchRight - fore_wav.fact.dwPitchRight;
    if (right_diff < 0) {
      for (int j=1; j<=aft_wav.fact.dwPitchRight; j++)
        morph_wav_data[j+aft_wav.fact.dwPitchLeft] = 
          fore_wav_data[j+fore_wav.fact.dwPitchLeft]*filter[i]
          + aft_wav_data[j+aft_wav.fact.dwPitchLeft]*(1-filter[i]);
    } else {
      vector<short> tmp_fore_wav_data(fore_wav_data);
      tmp_fore_wav_data.insert(tmp_fore_wav_data.end(), right_diff, 0);
      for (int j=1; j<=aft_wav.fact.dwPitchRight; j++)
        morph_wav_data[j+aft_wav.fact.dwPitchLeft] =
          tmp_fore_wav_data[j+fore_wav.fact.dwPitchLeft]*filter[i]
          + aft_wav_data[j+aft_wav.fact.dwPitchLeft]*(1-filter[i]);
    }

    base_wavs[base_pos+i].data.setData(normalize(morph_wav_data, target_rms));
  }

  cout << "----- finish making base wavs -----" << endl << endl;
  return true;
}

double BaseWavsMaker::getRMS(vector<short> wav)
{
  double rms = 0.0;
  for (int i=0; i<wav.size(); i++)
    rms += pow((double)wav[i], 2) / wav.size();
  return sqrt(rms);
}

vector<short> BaseWavsMaker::normalize(vector<short> wav, double target_rms)
{
  double wav_rms = getRMS(wav);
  for (int i=0; i<wav.size(); i++)
    wav[i] = wav[i] * (target_rms/wav_rms);
  return wav;
}

vector<double> BaseWavsMaker::getTri(long len)
{
  vector<double> filter(len, 0);

  for (int i=0; i<filter.size(); ++i) {
    double x = (i+1.0) / (filter.size()+1.0);
    filter[i] = 1.0 - 2*fabs(x-0.5);
  }

  return filter;
}

vector<double> BaseWavsMaker::getHann(long len)
{
  vector<double> filter(len, 0);

  for (int i=0; i<filter.size(); ++i) {
    double x = (i+1.0) / (filter.size()+1.0);
    filter[i] = 0.5 - (0.5 * cos(2*M_PI*x));
  }

  return filter;
}

vector<double> BaseWavsMaker::getLanczos(long len, unsigned short lobe)
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

double BaseWavsMaker::sinc(double x)
{
  return sin(M_PI*x)/(M_PI*x);
}
