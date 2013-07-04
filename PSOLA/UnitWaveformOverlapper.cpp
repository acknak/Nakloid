#include "UnitWaveformOverlapper.h"

using namespace std;
using namespace uw;

UnitWaveformOverlapper::UnitWaveformOverlapper(const WavFormat& format, const vector<float>& pitches):ms_margin(0)
{
  this->format = format;
  long tmp_ms = 0;
  list<long> tmp_pitchmarks(0);
  bool is_note_on = false;

  while (tmp_ms < pitches.size()) {
    if (pitches[tmp_ms] > 0) {
      if (!is_note_on)
        tmp_pitchmarks.push_back(nak::ms2pos(tmp_ms, format));
      tmp_pitchmarks.push_back(tmp_pitchmarks.back()+(1.0/pitches[tmp_ms]*format.dwSamplesPerSec));
      tmp_ms = nak::pos2ms(tmp_pitchmarks.back(), format);
      is_note_on = true;
    } else {
      tmp_ms++;
      is_note_on = false;
    }
  }

  this->pitchmarks.assign(tmp_pitchmarks.begin(), tmp_pitchmarks.end());
  output_wav.assign(pitchmarks.back(), 0);
}


UnitWaveformOverlapper::~UnitWaveformOverlapper(){}

bool UnitWaveformOverlapper::overlapping(const UnitWaveformContainer* const uwc, long pron_start, long pron_end, const vector<short>& velocities)
{
  if (pron_start < -ms_margin) {
    ms_margin += -pron_start;
    output_wav.insert(output_wav.begin(), nak::ms2pos(ms_margin, format), 0);
  }
  long ms_start=pron_start+ms_margin, ms_end=pron_end+ms_margin;
  if (pitchmarks.empty()) {
    cerr << "[UnitWaveformOverlapper::overlapping] pitchmarks not found" << endl;
    return false;
  }
  if (ms_start >= ms_end) {
    cerr << "[UnitWaveformOverlapper::overlapping] ms_start >= ms_end" << endl;
    return false;
  }
  if (uwc->unit_waveforms.empty()) {
    cerr << "[UnitWaveformOverlapper::overlapping] unit waveforms not found" << endl;
    return false;
  }

  long fade_start = (uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart-1)->fact.dwPosition;
  long fade_last = uwc->unit_waveforms.back().fact.dwPosition;
  vector<long>::const_iterator it_begin_pitchmarks = pos2it(nak::ms2pos(ms_start,format));
  vector<long>::const_iterator it_end_pitchmarks = pos2it(nak::ms2pos(ms_end,format));
  vector<long>::const_iterator it_pitchmarks = it_begin_pitchmarks;

  while (it_pitchmarks != it_end_pitchmarks) {
    vector<UnitWaveform>::const_iterator it_unit_waveform = uwc->unit_waveforms.begin();
    vector<double> tmp_waveform;
    {
      // choose unit waveform for overlap
      long dist = *it_pitchmarks - *it_begin_pitchmarks;
      if (dist > fade_last) {
        dist = ((dist-fade_start)/((short)nak::fade_stretch)%(fade_last-fade_start)
          +(uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart)->fact.dwPosition);
      }
      while (++it_unit_waveform!=uwc->unit_waveforms.end() && (it_unit_waveform)->fact.dwPosition<dist);
      --it_unit_waveform;

      if (nak::interpolation) {
        long dist_fore=dist-it_unit_waveform->fact.dwPosition, dist_aft;
        vector<double> waveform_fore=it_unit_waveform->data.getData(), waveform_aft;
        if (it_unit_waveform==--uwc->unit_waveforms.end()) {
          dist_aft = (uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart)->fact.dwPosition - dist;
          waveform_aft = (uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart)->data.getData();
        } else {
          dist_aft = (it_unit_waveform+1)->fact.dwPosition - dist;
          waveform_aft = (it_unit_waveform+1)->data.getData();
        }

        if (waveform_fore.size() > waveform_aft.size()) {
          waveform_aft.insert(waveform_aft.begin(), (waveform_fore.size()-waveform_aft.size())/2, 0);
          waveform_aft.insert(waveform_aft.end(), waveform_fore.size()-waveform_aft.size(), 0);
        } else {
          waveform_fore.insert(waveform_fore.begin(), (waveform_aft.size()-waveform_fore.size())/2, 0);
          waveform_fore.insert(waveform_fore.end(), waveform_aft.size()-waveform_fore.size(), 0);
        }

        tmp_waveform.assign(waveform_fore.size(), 0.0);
        double scale_fore=dist_aft/((double)dist_fore+dist_aft), scale_aft=1.0-scale_fore;
        for (size_t i=0; i<waveform_fore.size(); i++) {
          tmp_waveform[i] = (waveform_fore[i]*scale_fore) + (waveform_aft[i]*scale_aft);
        }
      } else {
        tmp_waveform = it_unit_waveform->data.getData();
      }
    }

    // overlap
    long win_start = *it_pitchmarks - it_unit_waveform->fact.dwPitchLeft;
    long win_end = *it_pitchmarks + it_unit_waveform->fact.dwPitchRight;
    if (win_start < 0) {
      // left edge
      tmp_waveform.erase(tmp_waveform.begin(), tmp_waveform.begin()-win_start);
      win_start = 0;
    }
    if (win_end >= pitchmarks.back()) {
      // right edge
      tmp_waveform.erase(tmp_waveform.end()-(win_end-pitchmarks.back()), tmp_waveform.end());
      win_end = pitchmarks.back();
    }
    long ms_dist = nak::pos2ms(*it_pitchmarks-*it_begin_pitchmarks, format);
    double scale = ((ms_dist<velocities.size())?velocities[ms_dist]:velocities.back())/100.0;
    for (size_t i=0; i<win_end-win_start; i++)
      output_wav[win_start+i] += tmp_waveform[i] * scale;

    ++it_pitchmarks;
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

void UnitWaveformOverlapper::outputWav(const wstring& output, long ms_margin)
{
  if (ms_margin > this->ms_margin) {
    output_wav.insert(output_wav.begin(), nak::ms2pos(ms_margin-this->ms_margin, format), 0);
  }
  outputWav(output);
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
