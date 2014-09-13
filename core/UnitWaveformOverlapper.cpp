#include "UnitWaveformOverlapper.h"

using namespace std;

struct UnitWaveformOverlapper::Parameters UnitWaveformOverlapper::params;

UnitWaveformOverlapper::UnitWaveformOverlapper(const vector<long>& pitchmarks): pitchmarks(pitchmarks)
{
  output_wav.assign(pitchmarks.back(), 0);
}

UnitWaveformOverlapper::~UnitWaveformOverlapper(){}

bool UnitWaveformOverlapper::overlapping(const UnitWaveformContainer* const uwc, pair<long, long> ms_note_pron, const vector<short>& velocities)
{
  return overlapping(uwc, ms_note_pron, 0, velocities);
}

bool UnitWaveformOverlapper::overlapping(const UnitWaveformContainer* const uwc, pair<long, long> ms_note_pron, long ms_note_margin, const vector<short>& velocities)
{
  if (ms_note_pron.second < 0) {
    cerr << "[UnitWaveformOverlapper::overlapping] negative ms_note_pron end" << endl;
    return false;
  }
  if (pitchmarks.empty()) {
    cerr << "[UnitWaveformOverlapper::overlapping] pitchmarks not found" << endl;
    return false;
  }
  if (uwc==0 || uwc->unit_waveforms.empty()) {
    cerr << "[UnitWaveformOverlapper::overlapping] unit waveforms not found" << endl;
    return false;
  }

  long ms_trim = (ms_note_pron.first<0)?-ms_note_pron.first:0;
  long ms_start=ms_note_pron.first+ms_trim, ms_end=ms_note_pron.second;
  if (ms_start >= ms_end) {
    cerr << "[UnitWaveformOverlapper::overlapping] ms_start >= ms_end" << endl;
    return false;
  }

  vector<PitchMarkObject> pmos;
  long fade_start=(uwc->unit_waveforms.begin()+uwc->header.dwRepeatStart-1)->dwPosition, fade_last=uwc->unit_waveforms.back().dwPosition;
  long pos_trim=ms2pos(ms_trim, params.wav_header), pos_margin=ms2pos(ms_note_margin,params.wav_header);
  double fade_stretch_scale = (params.self_fade)
    ? params.ms_self_fade/((uwc->unit_waveforms.back().dwPosition-uwc->unit_waveforms[uwc->header.dwRepeatStart].dwPosition)/(double)uwc->header.dwSamplesPerSec)
    : 1.0;
  vector<long>::const_iterator it_begin_pitchmarks=pos2it(ms2pos(ms_start,params.wav_header)), it_end_pitchmarks=pos2it(ms2pos(ms_end,params.wav_header));
  for (vector<long>::const_iterator it_pitchmarks=it_begin_pitchmarks;it_pitchmarks!=it_end_pitchmarks;++it_pitchmarks) {
    PitchMarkObject pmo(it_pitchmarks);
    vector<UnitWaveform>::const_iterator it_unit_waveform = uwc->unit_waveforms.begin();

    // choose unit waveform for overlap
    long dist = *it_pitchmarks - *it_begin_pitchmarks + pos_trim + pos_margin;
    if (dist > fade_last) {
      dist = ((long)((dist-fade_start)/fade_stretch_scale)%(fade_last-fade_start)
        +(uwc->unit_waveforms.begin()+uwc->header.dwRepeatStart)->dwPosition);
    }
    it_unit_waveform = binary_pos_search(it_unit_waveform, uwc->unit_waveforms.end(), dist);

    long output_pitch = (it_pitchmarks==it_begin_pitchmarks)?(*(it_pitchmarks+1)-*it_pitchmarks):(*it_pitchmarks-*(it_pitchmarks-1));
    long base_pitch = uwc->header.dwSamplesPerSec / uwc->header.wF0;
    pmo.uwps.push_back(PitchMarkObject::UnitWaveformParams(it_unit_waveform, 1.0, output_pitch, base_pitch, uwc->header.wLobeSize));
    if (params.interpolation) {
      // add data for interpolation to pmo
      long dist_fore=dist-it_unit_waveform->dwPosition, dist_aft;
      vector<UnitWaveform>::const_iterator it_aft_unit_waveform;
      if (it_unit_waveform==--uwc->unit_waveforms.end()) {
        it_aft_unit_waveform = uwc->unit_waveforms.begin()+uwc->header.dwRepeatStart;
        dist_aft = it_aft_unit_waveform->dwPosition - dist;
      } else {
        it_aft_unit_waveform = it_unit_waveform+1;
        dist_aft = (it_unit_waveform+1)->dwPosition - dist;
      }
      if (dist_fore > 0) {
        pmo.uwps[0].scale = dist_aft/(double)(dist_fore+dist_aft);
        pmo.uwps.push_back(PitchMarkObject::UnitWaveformParams(it_aft_unit_waveform,dist_fore/(double)(dist_fore+dist_aft), output_pitch, base_pitch, uwc->header.wLobeSize));
      }
    }
    pmos.push_back(pmo);
  }

  if (params.overlap_normalize) {
    long margin = 0;
    vector<double> subset_wav;
    if (params.overlap_normalize) {
      margin = uwc->unit_waveforms.front().dwPitchLeft;
      subset_wav.assign(ms2pos(ms_end-ms_start,params.wav_header)+margin+uwc->unit_waveforms.back().dwPitchRight,0);
    }
    // prepare subset wav
    for (vector<PitchMarkObject>::iterator it_pmos=pmos.begin();it_pmos!=pmos.end();++it_pmos) {
      long tmp_pos = *it_pmos->it - *it_begin_pitchmarks - it_pmos->uwps[0].uw.dwPitchLeft + margin;
      for (vector<PitchMarkObject::UnitWaveformParams>::iterator it_uwps=it_pmos->uwps.begin(); it_uwps!=it_pmos->uwps.end(); ++it_uwps) {
        vector<double> tmp_uwd = it_uwps->uw.data.getData();
        for (size_t i=0; i< tmp_uwd.size(); i++) {
          if (tmp_pos+i>=0 && tmp_pos+i<subset_wav.size()) {
            subset_wav[tmp_pos+i] += tmp_uwd[i] * it_uwps->scale;
          }
        }
      }
    }
    // analyze subset
    for (vector<PitchMarkObject>::iterator it_pmos=pmos.begin();it_pmos!=pmos.end();++it_pmos) {
      double mean_rms=it_pmos->getRmsMean();
      long tmp_pos = *it_pmos->it - *it_begin_pitchmarks + margin;
      if (tmp_pos-it_pmos->uwps[0].uw.dwPitchLeft >= 0) {
        vector<double> tmp_wav(subset_wav.begin()+tmp_pos-it_pmos->uwps.front().uw.dwPitchLeft, subset_wav.begin()+tmp_pos+it_pmos->uwps.front().uw.dwPitchRight+1);
        vector<double> filter = getWindow(tmp_wav.size(), params.num_lobes, it_pmos->uwps[0].uw.dwPitchLeft);
        for (size_t i=0; i<tmp_wav.size(); i++) {
          tmp_wav[i] *= filter[i];
        }
        double tmp_rms = WavData(tmp_wav).getRMS();
        it_pmos->scale = (tmp_rms>0.0)?(mean_rms/tmp_rms):0.0;
      } else {
        it_pmos->scale = 0.0;
      }
    }
  }

  // overlap
  for (vector<PitchMarkObject>::iterator it_pmos=pmos.begin(); it_pmos!=pmos.end(); ++it_pmos) {
    for (vector<PitchMarkObject::UnitWaveformParams>::iterator it_uwps=it_pmos->uwps.begin(); it_uwps!=it_pmos->uwps.end(); ++it_uwps) {
      long tmp_pos=*(it_pmos->it)-it_uwps->uw.dwPitchLeft, sub_velocity=pos2ms(*(it_pmos->it)-*it_begin_pitchmarks, params.wav_header);
      if (sub_velocity >= velocities.size()) {
        sub_velocity = velocities.back();
      }
      if (velocities[sub_velocity] > 0) {
        vector<double> tmp_uwd = it_uwps->uw.data.getData();
        for (long i=0; i< tmp_uwd.size(); i++) {
          if (tmp_pos+i>=0 && tmp_pos+i<output_wav.size()) {
            output_wav[tmp_pos+i] += tmp_uwd[i] * it_uwps->scale * (velocities[sub_velocity]/100.0) * it_pmos->scale;
          }
        }
      }
    }
  }

  return true;
}

void UnitWaveformOverlapper::outputWav(const boost::filesystem::path& path_output) const
{
  vector<double> tmp_output_wav(output_wav);
  cout << "normalize output wav" << endl << endl;
  double tmp_scale = params.max_volume / max(*max_element(output_wav.begin(), output_wav.end()),-*min_element(output_wav.begin(), output_wav.end()));
  for (size_t i=0; i<output_wav.size(); i++) {
    tmp_output_wav[i] *= tmp_scale;
  }
  if (params.compressor) {
    cout << "compress output wav" << endl << endl;
    for (size_t i=0; i<output_wav.size(); i++) {
      pair<bool, double> tmp_dB = val2dB(output_wav[i]);
      if (tmp_dB.second<1.0 && tmp_dB.second>params.compressor_threshold) {
        tmp_dB.second -= (tmp_dB.second - params.compressor_threshold) / params.compressor_ratio;
        tmp_output_wav[i] = dB2val(tmp_dB);
      }
    }
  }
  Wav wav(params.wav_header, tmp_output_wav);
  wav.save(path_output);
}

/*
 * accessor
 */
const vector<long>& UnitWaveformOverlapper::getPitchmarks() const
{
  return pitchmarks;
}

/*
 * private
 */
vector<long>::const_iterator UnitWaveformOverlapper::pos2it(long pos) const
{
  vector<long>::const_iterator it = pitchmarks.begin();
  do
    if (*it > pos)
      return it;
  while (++it != pitchmarks.end());
  return pitchmarks.end();
}

vector<UnitWaveform>::const_iterator UnitWaveformOverlapper::binary_pos_search(vector<UnitWaveform>::const_iterator from, vector<UnitWaveform>::const_iterator to, const long pos_target) const
{
  if (to-from > 1) {
    vector<UnitWaveform>::const_iterator it_half = (to-from) / 2 + from;
    if (it_half->dwPosition > pos_target) {
      return binary_pos_search(from, it_half, pos_target);
    } else if (it_half->dwPosition < pos_target) {
      return binary_pos_search(it_half, to, pos_target);
    }
    return it_half;
  }
  return from;
}

double UnitWaveformOverlapper::PitchMarkObject::getRmsMean()
{
  double sum = 0.0;
  for (vector<UnitWaveformParams>::const_iterator it_uwps=uwps.begin(); it_uwps!=uwps.end(); ++it_uwps) {
    sum += it_uwps->uw.data.getRMS() * it_uwps->scale;
  }
  return sum;
}

UnitWaveformOverlapper::PitchMarkObject::UnitWaveformParams::UnitWaveformParams(vector<UnitWaveform>::const_iterator it, double scale, long output_pitch, long base_pitch, unsigned char lobe)
  :scale(scale)
{
  uw.dwPosition = it->dwPosition;
  uw.dwPitchLeft = it->dwPitchLeft;
  uw.dwPitchRight = it->dwPitchRight;
  vector<double> base_waveform = it->data.getData();
  vector<double> filter = (params.window_modification&&base_pitch>output_pitch)?getWindow(base_waveform.size(),lobe,output_pitch):getWindow(base_waveform.size(),lobe);
  for (size_t i=0; i<base_waveform.size(); i++) {
    base_waveform[i] *= filter[i];
  }
  uw.data = WavData(base_waveform);
}
