#include "UnitWaveformMaker.h"

using namespace std;

struct UnitWaveformMaker::Parameters UnitWaveformMaker::params;

bool UnitWaveformMaker::makeUnitWaveform(const vector<double>& voice, bool is_vcv)
{
  return makeUnitWaveform(voice, -1, is_vcv);
}

bool UnitWaveformMaker::makeUnitWaveform(const vector<double>& voice, short pitch, bool is_vcv)
{
  this->voice = voice;
  if (voice.size()==0 || pitchmarks.empty() || params.num_lobes==0) {
    cerr << "[UnitWaveformMaker::makeUnitWaveform] voice, pitch mark or lobe is null" << endl;
    return false;
  }
  double fade_start_rms = makeUnitWaveform(sub_fade_start, pitch).data.getRMS();

  // make unit waveforms
  {
    double fade_scale=1.0, ovrl_scale=1.0;
    vector<double> base_wav;
    if (params.normalize) {
      fade_scale = params.target_rms/fade_start_rms;
      ovrl_scale = is_vcv?((sub_ovrl>0)?params.target_rms/makeUnitWaveform(0, pitch).data.getRMS():1):fade_scale;
    }
    for (size_t i=0; i<pitchmarks.size()&&i<sub_fade_end; i++) {
      double scale = 1.0;
      // adjust uwc volume
      if (params.normalize) {
        if (is_vcv) {
          if (i > sub_ovrl && i < sub_fade_start) {
            double tmp = (i-sub_ovrl) / (double)(sub_fade_start-sub_ovrl);
            scale = (ovrl_scale*(1.0-tmp)) + (fade_scale*tmp);
          } else if (i <= sub_ovrl) {
            scale = ovrl_scale;
          } else {
            scale = fade_scale;
          }
        } else {
          if (sub_ovrl==0 || i>=sub_fade_start) {
            scale = fade_scale;
          } else if (i <= sub_ovrl) {
            scale = ovrl_scale;
          } else {
            double tmp = (i-sub_ovrl) / (double)(sub_fade_start-sub_ovrl);
            scale = (ovrl_scale*(1.0-tmp)) + (fade_scale*tmp);
          }
        }
      }
      // make unit waveform
      uwc->unit_waveforms.push_back(makeUnitWaveform(i, pitch, scale));
      if (i > sub_fade_start+params.min_repeat_length) {
        if (base_wav.size() == 0) {
          base_wav = uwc->unit_waveforms[sub_fade_start].data.getData();
        }
        vector<double> target_wav = uwc->unit_waveforms.back().data.getData();
      }
    }
  }
  // make self fade
  {
    long sub_rep_len=sub_fade_end-sub_fade_start;
    if (params.repeat_type == Parameters::RepeatType::repeat_type_self_fade) {
      sub_rep_len = (sub_rep_len+1) / 2;
      sub_fade_start = sub_fade_end - (sub_rep_len*2);
    }
    UnitWaveform uw_dimin, uw_cres=uwc->unit_waveforms[sub_fade_start];
    for (size_t i=0; i<sub_rep_len; i++) {
      vector<double> data_cres, data_dimin;
      if (params.repeat_type == Parameters::RepeatType::repeat_type_self_fade) {
        uw_dimin = uwc->unit_waveforms[sub_fade_start+sub_rep_len+i];
        uw_cres = uwc->unit_waveforms[sub_fade_start+i];
      } else {
        uw_dimin = uwc->unit_waveforms[sub_fade_start+i];
      }
      data_cres = uw_cres.data.getData();
      data_dimin = uw_dimin.data.getData();
      {
        long left_diff=uw_dimin.dwPitchLeft-uw_cres.dwPitchLeft, right_diff=uw_dimin.dwPitchRight-uw_dimin.dwPitchRight;
        if (left_diff < 0) {
          data_cres.erase(data_cres.begin(), data_cres.begin()-left_diff);
        } else if (left_diff > 0) {
          data_cres.insert(data_cres.begin(), left_diff, 0);
        }
        if (right_diff < 0) {
          data_cres.erase(data_cres.end()+right_diff, data_cres.end());
        } else if (right_diff > 0) {
          data_cres.insert(data_cres.end(), right_diff, 0);
        }
      }
      {
        double scale = i / (double)sub_rep_len;
        for (size_t j=0; j<data_dimin.size(); j++) {
          data_dimin[j] = (data_cres[j]*scale) + (data_dimin[j]*(1.0-scale));
        }
      }
      if (params.repeat_type == Parameters::RepeatType::repeat_type_self_fade) {
        uwc->unit_waveforms[sub_fade_start+sub_rep_len+i].data.setData(data_dimin);
        uwc->header.dwRepeatStart = sub_fade_start+sub_rep_len;
      } else {
        uwc->unit_waveforms[sub_fade_start+i].data.setData(data_dimin);
        uwc->header.dwRepeatStart = sub_fade_start;
      }
    }
  }
  return true;
}

/*
 * accessor
 */
const vector<long>& UnitWaveformMaker::getPitchMarks() const
{
  return pitchmarks;
}

void UnitWaveformMaker::setOvrl(long ms_ovrl, unsigned long fs)
{
  long pos_ovrl = ms_ovrl * fs / 1000;
  for (size_t i=0; i<pitchmarks.size(); i++) {
    if (pos_ovrl <= pitchmarks[i]) {
      sub_ovrl = i;
      break;
    }
  }
}

void UnitWaveformMaker::setOvrl(long sub_ovrl)
{
  this->sub_ovrl = sub_ovrl;
}

void UnitWaveformMaker::setFadeParams(long ms_fade_start, long ms_fade_end, unsigned long fs)
{
  long pos_fade_start = ms_fade_start * fs / 1000;
  for (size_t i=0; i<pitchmarks.size(); i++) {
    if (pos_fade_start <= pitchmarks[i]) {
      sub_fade_start = i;
      break;
    }
  }
  long pos_fade_end = ms_fade_end * fs / 1000;
  if (pos_fade_end>pitchmarks.back() || ms_fade_start>ms_fade_end) {
    sub_fade_end = pitchmarks.size()-1;
  } else {
    for (size_t i=0; i<pitchmarks.size(); i++) {
      if (pos_fade_end <= pitchmarks[i]) {
        ms_fade_end = i;
        break;
      }
    }
  }
}

void UnitWaveformMaker::setFadeParams(long sub_fade_start, long sub_fade_end)
{
  this->sub_fade_start = sub_fade_start-((sub_fade_start==sub_fade_end)?1:0);
  this->sub_fade_end = sub_fade_end;
}

/*
 * private
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
  long win_start = pitchmarks[point] - (pitch*params.num_lobes);
  long win_end = pitchmarks[point] + (pitch*params.num_lobes);
  unit_waveform.dwPitchLeft = unit_waveform.dwPitchRight = pitch * params.num_lobes;
  unit_waveform.dwPosition = pitchmarks[point] - pitchmarks[0];

  // set unit waveform data
  vector<double> unit_waveform_data(win_end-win_start+1, 0);
  for (long i=0; i<unit_waveform_data.size(); i++) {
    unit_waveform_data[i] = (win_start+i<0 || win_start+i>=voice.size())?0:(voice[win_start+i])*scale;
  }
  unit_waveform.data.setData(unit_waveform_data);

  return unit_waveform;
}
