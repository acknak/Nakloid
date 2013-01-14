#include "BaseWavsMaker.h"

using namespace std;

BaseWavsMaker::BaseWavsMaker():lobe(nak::base_wavs_lobe),pos_rep_start(0),pos_offs(0),pos_blnk(0),sub_start(0),sub_end(0),sub_rep_start(0){}

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
  long sub_rep_len = base_wavs.size() - 1 - (sub_rep_start-sub_start);
  long base_pos = base_wavs.size() - 1 - (sub_rep_len/2);
  return base_pos;
}

void BaseWavsMaker::setRepStart(long ms_rep_start, unsigned long fs)
{
  this->pos_rep_start = ms_rep_start * fs / 1000;
}

void BaseWavsMaker::setRange(unsigned short ms_offs, short ms_blnk, unsigned long fs)
{
  this->pos_offs = ms_offs / 1000.0 * fs;
  this->pos_blnk = ms_blnk / 1000.0 * fs;
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
  if (pos_blnk > 0) {
    for (int i=0; i<pitches.size(); i++) {
      if (voice.size()-pos_blnk < (sub_tmp+=pitches[i])) {
        sub_end = i;
        break;
      }
    }
  } else {
    for (int i=0; i<pitches.size(); i++) {
      if (pos_offs - pos_blnk < (sub_tmp+=pitches[i])) {
        sub_end = i;
        break;
      }
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
  double target_rms = nak::getRMS(target);
  //short target_max = *max_element(target.begin(),target.end());
  //short target_min = *min_element(target.begin(),target.end());
  base_wavs.reserve(sub_end-sub_start);
  for (int i=sub_start; i<sub_end; i++) {
    BaseWav tmp_base_wav = makeBaseWav(pitches, i);
    tmp_base_wav.fact.dwPosition -= pitch_marks[sub_start];
    if (i > sub_rep_start)
      if (nak::is_normalize)
        tmp_base_wav.data.setData(nak::normalize(tmp_base_wav.data.getDataVector(),target_rms));
        //tmp_base_wav.data.setData(nak::normalize(tmp_base_wav.data.getDataVector(),target_max,target_min));
      else
        tmp_base_wav.data.setData(tmp_base_wav.data.getDataVector());
    base_wavs.push_back(tmp_base_wav);
  }

  // make self fade
  long sub_rep_len = base_wavs.size() - 1 - (sub_rep_start-sub_start);
  long base_pos = base_wavs.size() - 1 - (sub_rep_len/2);
  vector<double> filter = nak::getTri(sub_rep_len);
  filter.erase(filter.begin()+(filter.size()/2), filter.end());
  //double target_rms = nak::getRMS(base_wavs[sub_rep_start].data.getDataVector());
  //double target_mean = nak::getMean(base_wavs[sub_rep_start].data.getDataVector());
  //double target_var = nak::getVar(base_wavs[sub_rep_start].data.getDataVector(), target_mean);
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
  vector<double> filter = nak::getLanczos(pitches[point]*lobe*2+1 ,lobe);
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
