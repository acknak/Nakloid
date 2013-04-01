#include "BaseWavsMaker.h"

using namespace std;

BaseWavsMaker::BaseWavsMaker():lobe(nak::base_wavs_lobe),pos_rep_start(0),sub_rep_start(0){}

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

long BaseWavsMaker::getRepStartPoint()
{
  long sub_rep_len = base_wavs.size() - 1 - sub_rep_start;
  long base_pos = base_wavs.size() - 1 - (sub_rep_len/2);
  return base_pos;
}

void BaseWavsMaker::setRepStart(long ms_rep_start, unsigned long fs)
{
  this->pos_rep_start = ms_rep_start * fs / 1000;
}

bool BaseWavsMaker::makeBaseWavs()
{
  if (voice.size()==0 || pitch_marks.empty() || lobe==0)
    return false;
  base_wavs.clear();

  cout << "----- start making base wavs -----" << endl;

  // find self fade start point
  long sub_tmp = 0;
  for (int i=0; i<pitch_marks.size(); i++) {
    if (pos_rep_start < pitch_marks[i]) {
      sub_rep_start = i;
      break;
    }
  }

  // make base wavs
  vector<short> target = makeBaseWav(sub_rep_start).data.getDataVector();
  double target_rms = nak::getRMS(target);

  cout << "pitch_marks size: " << pitch_marks.size() << ", sub_rep_start:" << sub_rep_start << ", scale:" << 2400.0/target_rms << endl;

  base_wavs.reserve(pitch_marks.size());
  for (int i=0; i<pitch_marks.size(); i++) {
    BaseWav tmp_base_wav = makeBaseWav(i);
    if (i > sub_rep_start)
      tmp_base_wav.data.setData(nak::normalize(tmp_base_wav.data.getDataVector(), target_rms));
    base_wavs.push_back(tmp_base_wav);
  }

  // make self fade
  long sub_rep_len = (base_wavs.size()-1-sub_rep_start)/2;
  long base_pos = getRepStartPoint();

  cout << "base_wavs size:" << base_wavs.size() << ", base_pos:" << base_pos << endl;

  for (int i=0; i<sub_rep_len; i++) {
//for (int i=0; i<0; i++) {
    BaseWav fore_wav = base_wavs[sub_rep_start+i];
    BaseWav aft_wav = base_wavs[base_pos+i];

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

    //base_wavs[base_pos+i].data.setData(nak::normalize(aft_wav_data, 2400));
    base_wavs[base_pos+i].data.setData(aft_wav_data);
  }

  cout << "----- finish making base wavs -----" << endl << endl;

  return true;
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
