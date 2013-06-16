#include "UnitWaveformOverlapper.h"

using namespace std;
using namespace uw;

UnitWaveformOverlapper::UnitWaveformOverlapper(WavFormat format, list<float> pitches):ms_margin(0)
{
  vector<float> pitches_vector(pitches.begin(), pitches.end());
  UnitWaveformOverlapper(format, pitches_vector);
}

UnitWaveformOverlapper::UnitWaveformOverlapper(WavFormat format, vector<float> pitches):ms_margin(0)
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

bool UnitWaveformOverlapper::overlapping(const UnitWaveformContainer* uwc, long pron_start, long pron_end, vector<short> velocities)
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
  vector<long>::iterator it_begin_pitchmarks = pos2it(nak::ms2pos(ms_start,format));
  vector<long>::iterator it_end_pitchmarks = pos2it(nak::ms2pos(ms_end,format));
  vector<long>::iterator it_pitchmarks = it_begin_pitchmarks;

  while (it_pitchmarks != it_end_pitchmarks) {
    // choose unit waveform for overlap
    vector<UnitWaveform>::const_iterator it_unit_waveform = uwc->unit_waveforms.begin();
    long dist = *it_pitchmarks - *it_begin_pitchmarks;
    if (dist > fade_last) {
      dist = (fade_last==fade_start)?fade_start:((dist-fade_start)/((short)nak::fade_stretch)%(fade_last-fade_start)+fade_start);
    }
    while (it_unit_waveform->fact.dwPosition < dist)
      ++it_unit_waveform;

    // overlap
    vector<double> win = it_unit_waveform->data.getData();
    long win_start = *it_pitchmarks - it_unit_waveform->fact.dwPitchLeft;
    long win_end = *it_pitchmarks + it_unit_waveform->fact.dwPitchRight;
    if (win_start < 0) {
      // left edge
      win.erase(win.begin(), win.begin()-win_start);
      win_start = 0;
    }
    if (win_end >= pitchmarks.back()) {
      // right edge
      win.erase(win.end()-(win_end-pitchmarks.back()), win.end());
      win_end = pitchmarks.back();
    }
    long ms_dist = nak::pos2ms(*it_pitchmarks-*it_begin_pitchmarks, format);
    double scale = ((ms_dist<velocities.size())?velocities[ms_dist]:velocities.back())/100.0;
    for (int i=0; i<win_end-win_start; i++)
      output_wav[win_start+i] += win[i] * scale;

    ++it_pitchmarks;
  }

  return true;
}

void UnitWaveformOverlapper::outputWav(string output)
{
  cout << "normalize..." << endl << endl;
  double tmp_max_volume = max(*max_element(output_wav.begin(), output_wav.end()),*min_element(output_wav.begin(), output_wav.end())*-1);
  if (tmp_max_volume > nak::max_volume) {
    double tmp_scale = nak::max_volume / tmp_max_volume;
    for (int i=0; i<output_wav.size(); i++) {
      output_wav[i] *= tmp_scale;
    }
  }
  if (nak::compressor) {
    cout << "compressing..." << endl << endl;
    for (int i=0; i<output_wav.size(); i++) {
      pair<bool, double> tmp_dB = nak::val2dB(output_wav[i]);
      if (tmp_dB.second<1.0 && tmp_dB.second>nak::threshold) {
        tmp_dB.second -= (tmp_dB.second - nak::threshold) / nak::ratio;
        output_wav[i] = nak::dB2val(tmp_dB);
      }
    }
  }

  vector<short> output_wav_short(output_wav.size(), 0);
  WavParser::dbl2sht(&output_wav, &output_wav_short);
  long size = output_wav.size()*sizeof(short);
  ofstream ofs(output.c_str(), ios_base::out|ios_base::trunc|ios_base::binary);
  WavParser::setWavFileFormat(&ofs, format, size+28);
  ofs.write((char*)WavFormat::data, sizeof(char)*4);
  ofs.write((char*)&size, sizeof(long));
  ofs.write((char*)&output_wav_short[0], size);
  ofs.close();
}

void UnitWaveformOverlapper::outputWav(string output, long ms_margin)
{
  if (ms_margin > this->ms_margin) {
    output_wav.insert(output_wav.begin(), nak::ms2pos(ms_margin-this->ms_margin, format), 0);
  }
  outputWav(output);
}

vector<long>::iterator UnitWaveformOverlapper::pos2it(long pos)
{
  vector<long>::iterator it = pitchmarks.begin();
  do
    if (*it > pos)
      return it;
  while (++it != pitchmarks.end());
  return pitchmarks.end();
}


/*
 * accessor
 */
WavFormat UnitWaveformOverlapper::getWavFormat()
{
  return format;
}

list<long> UnitWaveformOverlapper::getPitchmarksList()
{
  list<long> pitchmarks(this->pitchmarks.begin(), this->pitchmarks.end());

  return pitchmarks;
}

vector<long> UnitWaveformOverlapper::getPitchmarksVector()
{
  return pitchmarks;
}
