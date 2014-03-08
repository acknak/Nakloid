#include "UnitWaveformMaker.h"

using namespace std;
using namespace uw;

UnitWaveformMaker::UnitWaveformMaker():lobe(nak::unit_waveform_lobe),sub_rep_start(0),sub_ovrl(0){}

UnitWaveformMaker::~UnitWaveformMaker(){}

bool UnitWaveformMaker::makeUnitWaveform(const vector<double>& voice, bool is_vcv)
{
  return makeUnitWaveform(voice, -1, is_vcv);
}

bool UnitWaveformMaker::makeUnitWaveform(const vector<double>& voice, short pitch, bool is_vcv)
{
  this->voice = voice;
  if (voice.size()==0 || pitchmarks.empty() || lobe==0) {
    cerr << "[UnitWaveformMaker] voice or pitch mark or lobe is null" << endl;
    return false;
  }
  if (sub_ovrl > sub_rep_start) {
    cerr << "[UnitWaveformMaker] invalid ovrl and rep_start" << endl;
    return false;
  }
  unit_waveforms.clear();

  // make unit waveforms
  long sub_fade_start = getFadeStartSub();
  {
    double fade_scale=1.0, ovrl_scale=1.0;
    if (nak::uwc_normalize) {
      fade_scale = nak::target_rms/makeUnitWaveform(sub_fade_start, pitch).data.getRMS();
      ovrl_scale = is_vcv?((sub_ovrl>0)?nak::target_rms/makeUnitWaveform(0, pitch).data.getRMS():1):fade_scale;
    }
    unit_waveforms.reserve(pitchmarks.size());
    for (size_t i=0; i<pitchmarks.size(); i++) {
      double scale = 1.0;
      if (nak::uwc_normalize) {
         if (is_vcv) {
          if (i > sub_ovrl && i < sub_fade_start) {
            double tmp = (i-sub_ovrl) / (double)(sub_fade_start-sub_ovrl);
            scale = (ovrl_scale*(1.0-tmp)) + (fade_scale*tmp);
          } else {
            scale = nak::target_rms/makeUnitWaveform(i, pitch).data.getRMS();
          }
        } else {
          if (sub_ovrl==0 || i>=sub_fade_start) {
            scale = nak::target_rms/makeUnitWaveform(i, pitch).data.getRMS();
          } else if (i <= sub_ovrl) {
            scale = ovrl_scale;
          } else {
            double tmp = (i-sub_ovrl) / (double)(sub_fade_start-sub_ovrl);
            scale = (ovrl_scale*(1.0-tmp)) + (fade_scale*tmp);
          }
        }
      }
      unit_waveforms.push_back(makeUnitWaveform(i, pitch, scale));
    }
  }

  // make self fade
  {
    long sub_rep_len = unit_waveforms.size() - sub_fade_start;
    double fade_start_rms = nak::getRMS(unit_waveforms[sub_fade_start].data.getData());
    for (size_t i=0; i<sub_rep_len; i++) {
      UnitWaveform fore_wav = unit_waveforms[sub_rep_start+i];
      UnitWaveform aft_wav = unit_waveforms[sub_fade_start+i];
      vector<double> fore_wav_data = fore_wav.data.getData();
      vector<double> aft_wav_data = aft_wav.data.getData();

      long left_diff = aft_wav.fact.dwPitchLeft - fore_wav.fact.dwPitchLeft;
      if (left_diff < 0) {
        fore_wav_data.erase(fore_wav_data.begin(), fore_wav_data.begin()-left_diff);
      } else if (left_diff > 0) {
        fore_wav_data.insert(fore_wav_data.begin(), left_diff, 0);
      }
      long right_diff = aft_wav.fact.dwPitchRight - fore_wav.fact.dwPitchRight;
      if (right_diff < 0) {
        fore_wav_data.erase(fore_wav_data.end()+right_diff, fore_wav_data.end());
      } else if (right_diff > 0) {
        fore_wav_data.insert(fore_wav_data.end(), right_diff, 0);
      }

      double scale = 1.0 / (sub_rep_len-1) * i;
      for (size_t j=0; j<aft_wav_data.size(); j++) {
        aft_wav_data[j] = (fore_wav_data[j]*scale) + (aft_wav_data[j]*(1.0-scale));
      }
      unit_waveforms[sub_fade_start+i].data.setData(nak::normalize(aft_wav_data, fade_start_rms));
    }
  }

  return true;
}

/*
 * accessor
 */
const vector<UnitWaveform>& UnitWaveformMaker::getUnitWaveform() const
{
  return unit_waveforms;
}

const vector<long>& UnitWaveformMaker::getPitchMarks() const
{
  return pitchmarks;
}

void UnitWaveformMaker::setPitchMarks(const vector<long>& pitchmarks)
{
  this->pitchmarks = pitchmarks;
  sub_rep_start = sub_ovrl = 0;
}

void UnitWaveformMaker::setPitchMarks(const vector<long>& pitchmarks, long ms_rep_start, unsigned long fs)
{
  setPitchMarks(pitchmarks, ms_rep_start, 0, fs);
}

void UnitWaveformMaker::setPitchMarks(const vector<long>& pitchmarks, long ms_rep_start, long ms_ovrl, unsigned long fs)
{
  this->pitchmarks = pitchmarks;

  long pos_rep_start = ms_rep_start * fs / 1000;
  for (size_t i=0; i<pitchmarks.size(); i++) {
    if (pos_rep_start <= pitchmarks[i]) {
      sub_rep_start = i;
      break;
    }
  }

  long pos_ovrl = ms_ovrl * fs / 1000;
  for (size_t i=0; i<pitchmarks.size(); i++) {
    if (pos_ovrl <= pitchmarks[i]) {
      sub_ovrl = i;
      break;
    }
  }
}

long UnitWaveformMaker::getFadeStartSub() const
{
  long tmp_fade_start = (pitchmarks.size()-1+sub_rep_start) / 2;
  return tmp_fade_start + (tmp_fade_start%2);
}

/*
 * protected
 */
UnitWaveform UnitWaveformMaker::makeUnitWaveform(short point, short pitch)
{
  return makeUnitWaveform(point, pitch, 1.0);
}

UnitWaveform UnitWaveformMaker::makeUnitWaveform(short point, short pitch, double scale)
{
  // cut window out
  UnitWaveform unit_waveform;
  if (pitch <= 0) {
    pitch = (point>=pitchmarks.size()-1)?pitchmarks.back()-(*----pitchmarks.end()):pitchmarks[point+1]-pitchmarks[point];
  }
  long win_start = pitchmarks[point] - (pitch*lobe);
  long win_end = pitchmarks[point] + (pitch*lobe);
  vector<double> filter = nak::getWindow(pitch*lobe*2+1, lobe);
  unit_waveform.fact.dwPitchLeft = unit_waveform.fact.dwPitchRight = pitch * lobe;
  unit_waveform.fact.dwPosition = pitchmarks[point] - pitchmarks[0];

  // set unit waveform data
  vector<double> unit_waveform_data(win_end-win_start+1, 0);
  for (long i=0; i<unit_waveform_data.size(); i++) {
    unit_waveform_data[i] = (win_start+i<0 || win_start+i>=voice.size())?0:(voice[win_start+i]*filter[i])*scale;
  }
  unit_waveform.data.setData(unit_waveform_data);

  return unit_waveform;
}
