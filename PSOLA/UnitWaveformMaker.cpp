#include "UnitWaveformMaker.h"

using namespace std;
using namespace uw;

UnitWaveformMaker::UnitWaveformMaker():lobe(nak::unit_waveform_lobe),sub_rep_start(0),sub_ovrl(0){}

UnitWaveformMaker::~UnitWaveformMaker(){}

vector<UnitWaveform> UnitWaveformMaker::getUnitWaveform()
{
  return unit_waveforms;
}

vector<long> UnitWaveformMaker::getPitchMarkVector()
{
  return pitch_marks;
}

list<long> UnitWaveformMaker::getPitchMarkList()
{
  list<long> pitch_marks(this->pitch_marks.begin(), this->pitch_marks.end());
  return pitch_marks;
}

void UnitWaveformMaker::setPitchMarks(vector<long> pitch_marks)
{
  this->pitch_marks = pitch_marks;
  sub_rep_start = sub_ovrl = 0;
}

void UnitWaveformMaker::setPitchMarks(vector<long> pitch_marks, long ms_rep_start, unsigned long fs)
{
  setPitchMarks(pitch_marks, ms_rep_start, 0, fs);
}

void UnitWaveformMaker::setPitchMarks(vector<long> pitch_marks, long ms_rep_start, long ms_ovrl, unsigned long fs)
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

long UnitWaveformMaker::getRepStartSub()
{
  return (unit_waveforms.size()-1+sub_rep_start) / 2;
}

bool UnitWaveformMaker::makeUnitWaveform(vector<short> voice, bool is_vcv)
{
  return makeUnitWaveform(voice, -1, is_vcv);
}

bool UnitWaveformMaker::makeUnitWaveform(vector<short> voice, short pitch, bool is_vcv)
{
  this->voice = voice;
  if (voice.size()==0 || pitch_marks.empty() || lobe==0) {
    cerr << "[UnitWaveformMaker] voice or pitch mark or lobe is null" << endl;
    return false;
  }
  if (sub_ovrl > sub_rep_start) {
    cerr << "[UnitWaveformMaker] invalid ovrl and rep_start" << endl;
    return false;
  }
  unit_waveforms.clear();

  // make unit waveforms
  double rep_scale = nak::target_rms/nak::getRMS(makeUnitWaveform(sub_rep_start, pitch).data.getDataVector());
  double ovrl_scale = (is_vcv&&sub_ovrl>0)?nak::target_rms/nak::getRMS(makeUnitWaveform(0, pitch).data.getDataVector()):rep_scale;
  unit_waveforms.reserve(pitch_marks.size());
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
    unit_waveforms.push_back(makeUnitWaveform(i, pitch, scale));
  }

  // make self fade
  long sub_base = getRepStartSub();
  long sub_rep_len = unit_waveforms.size() - sub_base;
  for (int i=0; i<sub_rep_len; i++) {
    UnitWaveform fore_wav = unit_waveforms[sub_rep_start+i];
    UnitWaveform aft_wav = unit_waveforms[sub_base+i];
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
    unit_waveforms[sub_base+i].data.setData(nak::normalize(aft_wav_data, nak::target_rms));
  }

  return true;
}

UnitWaveform UnitWaveformMaker::makeUnitWaveform(short point, short pitch)
{
  return makeUnitWaveform(point, pitch, 1.0);
}

UnitWaveform UnitWaveformMaker::makeUnitWaveform(short point, short pitch, double scale)
{
  // cut window out
  UnitWaveform unit_waveform;
  if (pitch <= 0) {
    pitch = (point>=pitch_marks.size()-1)?pitch_marks.back()-(*----pitch_marks.end()):pitch_marks[point+1]-pitch_marks[point];
  }
  long win_start = pitch_marks[point] - (pitch*lobe);
  long win_end = pitch_marks[point] + (pitch*lobe);
  vector<double> filter = nak::getLanczos(pitch*lobe*2+1 ,lobe);
  unit_waveform.fact.dwPitchLeft = unit_waveform.fact.dwPitchRight = pitch * lobe;
  unit_waveform.fact.dwPosition = pitch_marks[point] - pitch_marks[0];

  // set unit waveform data
  vector<short> unit_waveform_data(win_end-win_start+1, 0);
  for (int i=0; i<unit_waveform_data.size(); i++)
    unit_waveform_data[i] = (win_start+i<0 || win_start+i>=voice.size())?0:(voice[win_start+i]*filter[i])*scale;
  unit_waveform.data.setData(unit_waveform_data);

  return unit_waveform;
}
