#include "UnitWaveformOverlapper.h"

using namespace std;
using namespace uw;

UnitWaveformOverlapper::UnitWaveformOverlapper(const WavFormat& format, const vector<long>& pitchmarks)
  :format(format),pitchmarks(pitchmarks)
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
  if (uwc->unit_waveforms.empty()) {
    cerr << "[UnitWaveformOverlapper::overlapping] unit waveforms not found" << endl;
    return false;
  }

  long ms_trim = (ms_note_pron.first<0)?-ms_note_pron.first:0;
  long ms_start=ms_note_pron.first+ms_trim, ms_end=ms_note_pron.second;
  if (ms_start >= ms_end) {
    cerr << "[UnitWaveformOverlapper::overlapping] ms_start >= ms_end" << endl;
    return false;
  }

  long subset_wav_margin = 0;
  vector<double> subset_wav;
  if (nak::overlap_normalize) {
    // prepare subset
    subset_wav_margin = uwc->unit_waveforms.front().fact.dwPitchLeft;
    subset_wav.assign(nak::ms2pos(ms_end-ms_start,format)+subset_wav_margin+uwc->unit_waveforms.back().fact.dwPitchRight,0);
  }

  vector<PitchMarkObject> pmos;
  long fade_start=(uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart-1)->fact.dwPosition, fade_last=uwc->unit_waveforms.back().fact.dwPosition;
  long pos_trim=nak::ms2pos(ms_trim, format), pos_margin=nak::ms2pos(ms_note_margin,format);
  vector<long>::const_iterator it_begin_pitchmarks=pos2it(nak::ms2pos(ms_start,format)), it_end_pitchmarks=pos2it(nak::ms2pos(ms_end,format));
  for (vector<long>::const_iterator it_pitchmarks=it_begin_pitchmarks;it_pitchmarks!=it_end_pitchmarks;++it_pitchmarks) {
    PitchMarkObject pmo(it_pitchmarks);
    vector<UnitWaveform>::const_iterator it_unit_waveform = uwc->unit_waveforms.begin();

    // choose unit waveform for overlap
    long dist = *it_pitchmarks - *it_begin_pitchmarks + pos_trim + pos_margin;
    if (dist > fade_last) {
      dist = ((dist-fade_start)/((short)nak::fade_stretch)%(fade_last-fade_start)
        +(uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart)->fact.dwPosition);
    }
    it_unit_waveform = binary_pos_search(it_unit_waveform, uwc->unit_waveforms.end(), dist);

    PitchMarkObject::UnitWaveformSetting uws(it_unit_waveform, 1.0, nak::getRMS(it_unit_waveform->data.getData()));
    pmo.uwss.push_back(uws);
    if (nak::interpolation) {
      long dist_fore=dist-it_unit_waveform->fact.dwPosition, dist_aft;
      vector<UnitWaveform>::const_iterator it_aft_unit_waveform;
      if (it_unit_waveform==--uwc->unit_waveforms.end()) {
        it_aft_unit_waveform = uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart;
        dist_aft = it_aft_unit_waveform->fact.dwPosition - dist;
      } else {
        it_aft_unit_waveform = it_unit_waveform+1;
        dist_aft = (it_unit_waveform+1)->fact.dwPosition - dist;
      }
      PitchMarkObject::UnitWaveformSetting uws(it_aft_unit_waveform,((double)dist_fore+dist_aft)/dist_aft,nak::getRMS(it_aft_unit_waveform->data.getData()));
      pmo.uwss.push_back(uws);
    }
    if (nak::overlap_normalize) {
      // prepare subset wav
      for (vector<PitchMarkObject::UnitWaveformSetting>::iterator it_uwss=pmo.uwss.begin(); it_uwss!=pmo.uwss.end(); ++it_uwss) {
        long tmp_width=it_uwss->it->fact.dwPitchLeft+it_uwss->it->fact.dwPitchRight+1, tmp_pos=*it_pitchmarks-*it_begin_pitchmarks;
        double tmp_scale = it_uwss->scale / pmo.getRmsAccumulate();
        vector<double> tmp_uwd = it_uwss->it->data.getData();
        for (long i=0; i< tmp_width; i++) {
          if (tmp_pos+i>=0 && tmp_pos+i<subset_wav.size()) {
            subset_wav[tmp_pos+i] += tmp_uwd[i] * tmp_scale;
          }
        }
      }
    }
    pmos.push_back(pmo);
  }
  if (nak::overlap_normalize) {
    // analyze subset
    for (vector<PitchMarkObject>::iterator it_pmos=pmos.begin();it_pmos!=pmos.end();++it_pmos) {
      double acc_scale=it_pmos->getRmsAccumulate(), theo_rms=0.0;
      long tmp_width = it_pmos->uwss.front().it->fact.dwPitchLeft+it_pmos->uwss.front().it->fact.dwPitchRight+1;
      for (vector<PitchMarkObject::UnitWaveformSetting>::iterator it_uwss=it_pmos->uwss.begin(); it_uwss!=it_pmos->uwss.end(); ++it_uwss) {
        theo_rms += it_uwss->rms * it_uwss->scale / acc_scale;
      }
      vector<double> target_wav(subset_wav.begin()+(*it_pmos->it-*it_begin_pitchmarks), subset_wav.begin()+(*it_pmos->it-*it_begin_pitchmarks)+tmp_width);
      nak::multipleWindow(target_wav.begin(), target_wav.end(), nak::unit_waveform_lobe);
      it_pmos->scale = theo_rms / nak::getRMS(target_wav.begin(), target_wav.end());
    }
    vector<double>().swap(subset_wav);
  }

  // overlap
  for (vector<PitchMarkObject>::iterator it_pmos=pmos.begin(); it_pmos!=pmos.end(); ++it_pmos) {
    double acc_scale = it_pmos->getRmsAccumulate();
    for (vector<PitchMarkObject::UnitWaveformSetting>::iterator it_uwss=it_pmos->uwss.begin(); it_uwss!=it_pmos->uwss.end(); ++it_uwss) {
      long tmp_pos=*(it_pmos->it)-it_uwss->it->fact.dwPitchLeft, sub_velocity=nak::pos2ms(*(it_pmos->it)-*it_begin_pitchmarks, format);
      if (sub_velocity >= velocities.size()) {
        sub_velocity = velocities.back();
      }
      vector<double> tmp_uwd = it_uwss->it->data.getData();
      for (long i=0; i< tmp_uwd.size(); i++) {
        if (tmp_pos+i>=0 && tmp_pos+i<output_wav.size() && velocities[sub_velocity]!=0) {
          output_wav[tmp_pos+i] += tmp_uwd[i] * (it_uwss->scale/acc_scale) * (velocities[sub_velocity]/100.0) * it_pmos->scale;
        }
      }
    }
  }

  return true;
}

void UnitWaveformOverlapper::outputNormalization()
{
  cout << "normalize..." << endl << endl;
  double tmp_scale = nak::max_volume / max(*max_element(output_wav.begin(), output_wav.end()),-*min_element(output_wav.begin(), output_wav.end()));
  for (size_t i=0; i<output_wav.size(); i++) {
    output_wav[i] *= tmp_scale;
  }
}

void UnitWaveformOverlapper::outputCompressing()
{
  cout << "compressing..." << endl << endl;
  for (size_t i=0; i<output_wav.size(); i++) {
    pair<bool, double> tmp_dB = nak::val2dB(output_wav[i]);
    if (tmp_dB.second<1.0 && tmp_dB.second>nak::threshold) {
      tmp_dB.second -= (tmp_dB.second - nak::threshold) / nak::ratio;
      output_wav[i] = nak::dB2val(tmp_dB);
    }
  }
}

void UnitWaveformOverlapper::outputWav(const wstring& output) const
{
  boost::filesystem::ofstream ofs(output, ios_base::out|ios_base::trunc|ios_base::binary);
  WavParser::setWavFile(&ofs, format, &output_wav);
  ofs.close();
}

/*
 * accessor
 */
const WavFormat& UnitWaveformOverlapper::getWavFormat() const
{
  return format;
}

const vector<long>& UnitWaveformOverlapper::getPitchmarks() const
{
  return pitchmarks;
}

/*
 * protected
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
    if (it_half->fact.dwPosition > pos_target) {
      return binary_pos_search(from, it_half, pos_target);
    } else if (it_half->fact.dwPosition < pos_target) {
      return binary_pos_search(it_half, to, pos_target);
    }
    return it_half;
  }
  return from;
}

double UnitWaveformOverlapper::PitchMarkObject::getRmsAccumulate()
{
  double accumurate_scale = 0.0;
  for (vector<UnitWaveformSetting>::const_iterator it_uwss=uwss.begin(); it_uwss!=uwss.end(); ++it_uwss) {
    accumurate_scale += it_uwss->scale;
  }
  return accumurate_scale;
}
