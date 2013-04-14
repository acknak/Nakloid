#include "BaseWavsMaker.h"

using namespace std;

BaseWavsMaker::BaseWavsMaker():lobe(nak::base_wavs_lobe),sub_rep_start(0),sub_ovrl(0){}

BaseWavsMaker::~BaseWavsMaker(){}

vector<BaseWav> BaseWavsMaker::getBaseWavs()
{
  return base_wavs;
}

vector<long> BaseWavsMaker::getPitchMarkVector()
{
  return pitch_marks;
}

list<long> BaseWavsMaker::getPitchMarkList()
{
  list<long> pitch_marks(this->pitch_marks.begin(), this->pitch_marks.end());
  return pitch_marks;
}

void BaseWavsMaker::setPitchMarks(vector<long> pitch_marks)
{
  this->pitch_marks = pitch_marks;
  sub_rep_start = sub_ovrl = 0;
}

void BaseWavsMaker::setPitchMarks(vector<long> pitch_marks, long ms_rep_start, unsigned long fs)
{
  setPitchMarks(pitch_marks, ms_rep_start, 0, fs);
}

void BaseWavsMaker::setPitchMarks(vector<long> pitch_marks, long ms_rep_start, long ms_ovrl, unsigned long fs)
{
  this->pitch_marks = pitch_marks;

  long pos_rep_start = ms_rep_start * fs / 1000;
  for (int i=0; i<pitch_marks.size(); i++) {
    if (pos_rep_start <= pitch_marks[i]) {
      sub_rep_start = i;
      break;
    }
  }

  long pos_ovrl = ms_ovrl * fs / 1000;
  for (int i=0; i<pitch_marks.size(); i++) {
    if (pos_ovrl <= pitch_marks[i]) {
      sub_ovrl = i;
      break;
    }
  }
}

void BaseWavsMaker::setPitchMarks(list<long> pitch_marks)
{
  vector<long> tmp_pitch_marks(pitch_marks.begin(), pitch_marks.end());
  setPitchMarks(tmp_pitch_marks);
}

void BaseWavsMaker::setPitchMarks(list<long> pitch_marks, long ms_rep_start, unsigned long fs)
{
  vector<long> tmp_pitch_marks(pitch_marks.begin(), pitch_marks.end());
  setPitchMarks(tmp_pitch_marks, ms_rep_start, fs);
}

void BaseWavsMaker::setPitchMarks(list<long> pitch_marks, long ms_rep_start, long ms_ovrl, unsigned long fs)
{
  vector<long> tmp_pitch_marks(pitch_marks.begin(), pitch_marks.end());
  setPitchMarks(tmp_pitch_marks, ms_rep_start, ms_ovrl, fs);
}

long BaseWavsMaker::getRepStartSub()
{
  return (base_wavs.size()-1+sub_rep_start) / 2;
}

bool BaseWavsMaker::makeBaseWavs(vector<short> voice, bool is_vcv)
{
  this->voice = voice;
  if (voice.size()==0 || pitch_marks.empty() || lobe==0) {
    cerr << "[BaseWavsMaker] voice or pitch mark or lobe is null" << endl;
    return false;
  }
  if (sub_ovrl > sub_rep_start) {
    cerr << "[BaseWavsMaker] invalid ovrl and rep_start" << endl;
    return false;
  }
  base_wavs.clear();

  // make base wavs
  double rep_scale = nak::target_rms/nak::getRMS(makeBaseWav(sub_rep_start).data.getDataVector());
  double ovrl_scale = (is_vcv&&sub_ovrl>0)?nak::target_rms/nak::getRMS(makeBaseWav(0).data.getDataVector()):rep_scale;

  vector<double> filter(pitch_marks.size(), 1.0);
  base_wavs.reserve(pitch_marks.size());
  for (int i=0; i<pitch_marks.size(); i++) {
    double scale = 1.0;
    if (sub_ovrl==0 || i>=sub_rep_start) {
      scale = rep_scale;
    } else if (i <= sub_ovrl) {
      scale = ovrl_scale;
    } else {
      double tmp = (i-sub_ovrl) / (double)(sub_rep_start-sub_ovrl);
      scale = (ovrl_scale*(1.0-tmp)) + (rep_scale*tmp);
    }
    base_wavs.push_back(makeBaseWav(i, scale));
  }

  // make self fade
  long sub_base = getRepStartSub();
  long sub_rep_len = base_wavs.size() - sub_base;
  for (int i=0; i<sub_rep_len; i++) {
    BaseWav fore_wav = base_wavs[sub_rep_start+i];
    BaseWav aft_wav = base_wavs[sub_base+i];
    vector<short> fore_wav_data = fore_wav.data.getDataVector();
    vector<short> aft_wav_data = aft_wav.data.getDataVector();

    long left_diff = aft_wav.fact.dwPitchLeft - fore_wav.fact.dwPitchLeft;
    if (left_diff < 0) {
      fore_wav_data.erase(fore_wav_data.begin(), fore_wav_data.begin()-left_diff);
    } else {
      fore_wav_data.insert(fore_wav_data.begin(), left_diff, 0);
    }
    long right_diff = aft_wav.fact.dwPitchRight - fore_wav.fact.dwPitchRight;
    if (right_diff < 0) {
      fore_wav_data.erase(fore_wav_data.end()+right_diff, fore_wav_data.end());
    } else {
      fore_wav_data.insert(fore_wav_data.end(), right_diff, 0);
    }

    double scale = 1.0 / (sub_rep_len-1) * i;
    for (int j=0; j<aft_wav_data.size(); j++) {
      aft_wav_data[j] = (fore_wav_data[j]*scale) + (aft_wav_data[j]*(1.0-scale));
    }
    base_wavs[sub_base+i].data.setData(aft_wav_data);
  }

  return true;
}

bool BaseWavsMaker::makeBaseWavs(list<short> voice, bool is_vcv)
{
  vector<short> tmp_voice(voice.begin(), voice.end());
  return makeBaseWavs(tmp_voice, is_vcv);
}

BaseWav BaseWavsMaker::makeBaseWav(int point)
{
  return makeBaseWav(point, 1.0);
}

BaseWav BaseWavsMaker::makeBaseWav(int point, double scale)
{
  // cut window out
  BaseWav base_wav;
  long pitch = (point>=pitch_marks.size()-1)?pitch_marks.back()-(*----pitch_marks.end()):pitch_marks[point+1]-pitch_marks[point];
  long win_start = pitch_marks[point] - (pitch*lobe);
  long win_end = pitch_marks[point] + (pitch*lobe);
  vector<double> filter = nak::getLanczos(pitch*lobe*2+1 ,lobe);
  base_wav.fact.dwPitchLeft = base_wav.fact.dwPitchRight = pitch * lobe;
  base_wav.fact.dwPosition = pitch_marks[point] - pitch_marks[0];

  // set base wav data
  vector<short> base_wav_data(win_end-win_start+1, 0);
  for (int i=0; i<base_wav_data.size(); i++)
    base_wav_data[i] = (win_start+i<0 || win_start+i>=voice.size())?0:(voice[win_start+i]*filter[i])*scale;
  base_wav.data.setData(base_wav_data);

  return base_wav;
}
