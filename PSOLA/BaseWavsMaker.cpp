#include "BaseWavsMaker.h"

using namespace std;

BaseWavsMaker::BaseWavsMaker():lobe(1),pos_rep_start(0),pos_offs(0),pos_blnk(0),sub_start(0),sub_end(0),sub_rep_start(0){}

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

long BaseWavsMaker::getRepStartPoint()
{
  long sub_rep_len = base_wavs.size() - 1 - (sub_rep_start-sub_start);
  long base_pos = base_wavs.size() - 1 - (sub_rep_len/2);
  return base_pos;
}

void BaseWavsMaker::setRepStart(long ms_rep_start, unsigned long fs)
{
  this->pos_rep_start = ms_rep_start * fs / 1000;
}

void BaseWavsMaker::setRange(unsigned short ms_offs, unsigned short ms_blnk, unsigned long fs)
{
  this->pos_offs = ms_offs * fs / 1000;
  this->pos_blnk = ms_blnk * fs / 1000;
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

  // find base point
  long sub_tmp = 0;
  for (int i=0; i<pitches.size(); i++) {
    if (pos_offs < (sub_tmp+=pitches[i])) {
      sub_start = i;
      break;
    }
  }
  sub_tmp = 0;
  for (int i=0; i<pitches.size(); i++) {
    if (voice.size()-pos_blnk < (sub_tmp+=pitches[i])) {
      sub_end = i;
      break;
    }
  }
  sub_tmp = 0;
  for (int i=sub_start; i<sub_end; i++) {
    if (pos_rep_start < (sub_tmp+=pitches[i])) {
      sub_rep_start = i;
      if ((sub_end-1-sub_rep_start)%2 > 0)
        ++sub_rep_start;
      break;
    }
  }
  cout << "pitches size:" << pitches.size() 
    << ", sub_start:" << sub_start
    << ", sub_rep_start:" << sub_rep_start
    << ", sub_end:" << sub_end << endl;

  // make base wavs
  base_wavs.clear();
  vector<short> target = makeBaseWav(pitches,sub_rep_start).data.getDataVector();
  double target_rms = getRMS(target);
  //short target_max = *max_element(target.begin(),target.end());
  //short target_min = *min_element(target.begin(),target.end());
  base_wavs.reserve(sub_end-sub_start);
  for (int i=sub_start; i<sub_end; i++) {
    BaseWav tmp_base_wav = makeBaseWav(pitches, i);
    tmp_base_wav.fact.dwPosition -= pitch_marks[sub_start];
    if (i > sub_rep_start)
      //tmp_base_wav.data.setData(normalize(tmp_base_wav.data.getDataVector(),target_max,target_min));
      tmp_base_wav.data.setData(normalize(tmp_base_wav.data.getDataVector(),target_rms));
    base_wavs.push_back(tmp_base_wav);
  }

  // make self fade
  long sub_rep_len = base_wavs.size() - 1 - (sub_rep_start-sub_start);
  long base_pos = base_wavs.size() - 1 - (sub_rep_len/2);
  vector<double> filter = getTri(sub_rep_len);
  filter.erase(filter.begin()+(filter.size()/2), filter.end());
  //double target_rms = getRMS(base_wavs[sub_rep_start].data.getDataVector());
  //double target_mean = getMean(base_wavs[sub_rep_start].data.getDataVector());
  //double target_var = getVar(base_wavs[sub_rep_start].data.getDataVector(), target_mean);
  //vector<short> target = base_wavs[sub_rep_start].data.getDataVector();
  //short target_max = *max_element(target.begin(),test.end());
  //short target_min = *min_element(target.begin(),test.end());
  cout << "base_wavs size:" << base_wavs.size() << ", base_pos:" << base_pos << endl;
  for (int i=0; i<sub_rep_len/2; i++) {
    BaseWav fore_wav = base_wavs[(sub_rep_start-sub_start)+i];
    BaseWav aft_wav = base_wavs[base_pos+i];
    vector<short> fore_wav_data = fore_wav.data.getDataVector();
    vector<short> aft_wav_data = aft_wav.data.getDataVector();
    vector<short> fade_wav_data(aft_wav_data.size(), 0);

    // left
    long left_diff = aft_wav.fact.dwPitchLeft - fore_wav.fact.dwPitchLeft;
    if (left_diff < 0) {
      for (int j=0; j<aft_wav.fact.dwPitchLeft; j++) {
        fade_wav_data[j] = fore_wav_data[-left_diff+j]*filter[i] + aft_wav_data[j]*(1-filter[i]);
      }
    } else {
      vector<short> tmp_fore_wav_data(fore_wav_data);
      tmp_fore_wav_data.insert(tmp_fore_wav_data.begin(), left_diff, 0);
      for (int j=0; j<aft_wav.fact.dwPitchLeft; j++)
        fade_wav_data[j] = tmp_fore_wav_data[j]*filter[i] + aft_wav_data[j]*(1-filter[i]);
    }

    // center
    fade_wav_data[aft_wav.fact.dwPitchLeft] =
      fore_wav_data[fore_wav.fact.dwPitchLeft]*filter[i]
      + aft_wav_data[aft_wav.fact.dwPitchLeft]*(1-filter[i]);

    // right
    long right_diff = aft_wav.fact.dwPitchRight - fore_wav.fact.dwPitchRight;
    if (right_diff < 0) {
      for (int j=1; j<=aft_wav.fact.dwPitchRight; j++)
        fade_wav_data[j+aft_wav.fact.dwPitchLeft] = 
          fore_wav_data[j+fore_wav.fact.dwPitchLeft]*filter[i]
          + aft_wav_data[j+aft_wav.fact.dwPitchLeft]*(1-filter[i]);
    } else {
      vector<short> tmp_fore_wav_data(fore_wav_data);
      tmp_fore_wav_data.insert(tmp_fore_wav_data.end(), right_diff, 0);
      for (int j=1; j<=aft_wav.fact.dwPitchRight; j++)
        fade_wav_data[j+aft_wav.fact.dwPitchLeft] =
          tmp_fore_wav_data[j+fore_wav.fact.dwPitchLeft]*filter[i]
          + aft_wav_data[j+aft_wav.fact.dwPitchLeft]*(1-filter[i]);
    }
    //base_wavs[base_pos+i].data.setData(base_wavs[sub_rep_start].data.getDataVector());
    //base_wavs[base_pos+i].fact = base_wavs[sub_rep_start].fact;

    //base_wavs[base_pos+i].data.setData(normalize(fade_wav_data, target_max, target_min, target_mean));
    //base_wavs[base_pos+i].data.setData(normalize(fade_wav_data, target_mean, target_var));
    //base_wavs[base_pos+i].data.setData(normalize(fade_wav_data, target_rms));
    base_wavs[base_pos+i].data.setData(fade_wav_data);
  }

  cout << "----- finish making base wavs -----" << endl << endl;
  return true;
}

BaseWav BaseWavsMaker::makeBaseWav(vector<long> pitches, int point)
{
  // cut window out
  BaseWav base_wav;
  long win_start = pitch_marks[point] - (pitches[point]*lobe);
  long win_end = pitch_marks[point] + (pitches[point]*lobe);
  vector<double> filter = getLanczos(pitches[point]*lobe*2+1 ,lobe);
  base_wav.fact.dwPitchLeft = pitches[point] * lobe;
  base_wav.fact.dwPitchRight = pitches[point] * lobe;
  base_wav.fact.dwPosition = pitch_marks[point];

  // set base wav data
  vector<short> base_wav_data(win_end-win_start+1, 0);
  for (int j=0; j<base_wav_data.size(); j++)
    base_wav_data[j] = (win_start+j<0 || win_start+j>=voice.size())?0:(voice[win_start+j]*filter[j]);
  base_wav.data.setData(base_wav_data);

  return base_wav;
}

double BaseWavsMaker::getRMS(vector<short> wav)
{
  double rms = 0.0;
  for (int i=0; i<wav.size(); i++)
    rms += pow((double)wav[i], 2) / wav.size();
  return sqrt(rms);
}

double BaseWavsMaker::getMean(vector<short> wav)
{
  double mean = 0.0;
  for (int i=0; i<wav.size(); i++)
    mean += wav[i] / (double)wav.size();
  return mean;
}

double BaseWavsMaker::getVar(vector<short> wav, double mean)
{
  double var = 0.0;
  for (int i=0; i<wav.size(); i++)
    var += pow(wav[i]-mean, 2) / wav.size();
  return sqrt(var);
}

vector<short> BaseWavsMaker::normalize(vector<short> wav, double target_rms)
{
  double scale = target_rms / getRMS(wav);
  for (int i=0; i<wav.size(); i++)
    wav[i] = wav[i] * scale;
  return wav;
}

vector<short> BaseWavsMaker::normalize(vector<short> wav, double target_mean, double target_var)
{
  double wav_mean = getMean(wav);
  double wav_var = getVar(wav, wav_mean);
  for (int i=0; i<wav.size(); i++)
    wav[i] = (wav[i]+(target_mean-wav_mean)) * (target_var/wav_var);
  return wav;
}

vector<short> BaseWavsMaker::normalize(vector<short> wav, short target_max, short target_min)
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
