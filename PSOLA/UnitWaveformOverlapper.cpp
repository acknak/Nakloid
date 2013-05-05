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
  unsigned long tmp_ms = 0;
  list<unsigned long> tmp_pitchmarks(0);
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

  unsigned long fade_start = (uwc->unit_waveforms.begin()+uwc->format.dwRepeatStart)->fact.dwPosition;
  unsigned long fade_last = uwc->unit_waveforms.back().fact.dwPosition;
  vector<unsigned long>::iterator it_begin_pitchmarks = pos2it(nak::ms2pos(ms_start,format));
  vector<unsigned long>::iterator it_end_pitchmarks = pos2it(nak::ms2pos(ms_end,format));
  vector<unsigned long>::iterator it_pitchmarks = it_begin_pitchmarks;

  while (it_pitchmarks != it_end_pitchmarks) {
    // choose unit waveform for overlap
    vector<UnitWaveform>::const_iterator it_unit_waveform = uwc->unit_waveforms.begin();
    unsigned long dist = *it_pitchmarks - *it_begin_pitchmarks;
    if (dist > fade_start) {
      dist = (fade_last==fade_start)?fade_start:((dist-fade_start)/((short)nak::fade_stretch)%(fade_last-fade_start)+fade_start);
    }
    while (it_unit_waveform->fact.dwPosition < dist)
      ++it_unit_waveform;

    // overlap
    vector<short> win = it_unit_waveform->data.getDataVector();
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
    unsigned long ms_dist = nak::pos2ms(*it_pitchmarks-*it_begin_pitchmarks, format);
    double scale = ((ms_dist<velocities.size())?velocities[ms_dist]:velocities.back())/100.0;
    for (int i=0; i<win_end-win_start; i++)
      output_wav[win_start+i] += win[i] * scale;

    ++it_pitchmarks;
  }

  return true;
}

void UnitWaveformOverlapper::outputWav(string output)
{
  if (nak::compressor) {
    cout << "compressing..." << endl << endl;
    if (nak::max_volume > 1.0)
      nak::max_volume = 1.0;
    else if (nak::max_volume < 0)
      nak::max_volume = 0;
    long tmp_max_volume = max(*max_element(output_wav.begin(), output_wav.end()),*max_element(output_wav.begin(), output_wav.end())*-1);
    double tmp_max_db = log10((double)tmp_max_volume) * 20;
    double border_db_x = log10(pow(32767*nak::max_volume,2.0)) * 10 * nak::threshold_x;
    double slope_fore = nak::threshold_y / nak::threshold_x;
    double border_db_y = border_db_x * slope_fore;
    double slope_aft =  (border_db_y/nak::threshold_y-border_db_y) / (tmp_max_db-border_db_x);
    for (int i=0; i<output_wav.size(); i++) {
      if (output_wav[i] == 0)
        continue;
      double tmp_db = log10(pow(output_wav[i],2.0)) * 10;
      if (tmp_db < border_db_x)
        output_wav[i] *= pow(10, (tmp_db*slope_fore-tmp_db)/20);
      else
        output_wav[i] *= pow(10, ((tmp_db-border_db_x)*slope_aft+border_db_y-tmp_db)/20);
    }
  }
  vector<short> tmp_output_wav(output_wav.begin(), output_wav.end());
  long size = output_wav.size()*sizeof(short);
  ofstream ofs(output.c_str(), ios_base::out|ios_base::trunc|ios_base::binary);
  WavParser::setWavHeader(&ofs, format, size+28);
  ofs.write((char*)WavFormat::data, sizeof(char)*4);
  ofs.write((char*)&size, sizeof(long));
  ofs.write((char*)(&tmp_output_wav[0]), size);
  ofs.close();
}

void UnitWaveformOverlapper::outputWav(string output, unsigned long ms_margin)
{
  if (ms_margin > this->ms_margin) {
    output_wav.insert(output_wav.begin(), nak::ms2pos(ms_margin-this->ms_margin, format), 0);
  }
  outputWav(output);
}

vector<unsigned long>::iterator UnitWaveformOverlapper::pos2it(unsigned long pos)
{
  vector<unsigned long>::iterator it = pitchmarks.begin();
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

list<unsigned long> UnitWaveformOverlapper::getPitchmarksList()
{
  list<unsigned long> pitchmarks(this->pitchmarks.begin(), this->pitchmarks.end());

  return pitchmarks;
}

vector<unsigned long> UnitWaveformOverlapper::getPitchmarksVector()
{
  return pitchmarks;
}
