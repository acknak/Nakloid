#include "BaseWavsOverlapper.h"

using namespace std;

BaseWavsOverlapper::BaseWavsOverlapper(WavFormat format, list<float> pitches)
{
  vector<float> pitches_vector(pitches.begin(), pitches.end());
  BaseWavsOverlapper(format, pitches_vector);
}

BaseWavsOverlapper::BaseWavsOverlapper(WavFormat format, vector<float> pitches)
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


BaseWavsOverlapper::~BaseWavsOverlapper(){}

bool BaseWavsOverlapper::overlapping(Note note, Voice voice)
{
  long ms_start=note.getPronStart(), ms_end=note.getPronEnd();
  BaseWavsContainer bwc = voice.bwc;
  if (pitchmarks.empty()) {
    cerr << "[BaseWavsOverlapper::overlapping] pitchmarks not found" << endl;
    return false;
  }
  if (ms_start >= ms_end) {
    cerr << "[BaseWavsOverlapper::overlapping] ms_start >= ms_end" << endl;
    return false;
  }
  if (bwc.base_wavs.empty()) {
    cerr << "[BaseWavsOverlapper::overlapping] base_wavs not found" << endl;
    return false;
  }

  vector<short> velocities = note.getVelocities();
  unsigned long fade_start = (bwc.base_wavs.begin()+bwc.format.dwRepeatStart)->fact.dwPosition;
  unsigned long fade_last = bwc.base_wavs.back().fact.dwPosition;
  vector<unsigned long>::iterator it_begin_pitchmarks = pos2it(nak::ms2pos(ms_start,format));
  vector<unsigned long>::iterator it_end_pitchmarks = pos2it(nak::ms2pos(ms_end,format));
  vector<unsigned long>::iterator it_pitchmarks = it_begin_pitchmarks;

  if (note.isVCV()) {
    for (int i=0; i<note.getOvrl(); i++) {
      velocities[i] *= i/(double)note.getOvrl();
    }
  }

  while (it_pitchmarks != it_end_pitchmarks) {
    // choose overlap base_wav
    vector<BaseWav>::iterator it_base_wav = bwc.base_wavs.begin();
    unsigned long dist = *it_pitchmarks - *it_begin_pitchmarks;
    if (dist > fade_start) {
      dist = (fade_last==fade_start)?fade_start:((dist-fade_start)/((short)nak::fade_stretch)%(fade_last-fade_start)+fade_start);
    }
    while (it_base_wav->fact.dwPosition < dist)
      ++it_base_wav;

    // overlap
    vector<short> win = it_base_wav->data.getDataVector();
    long win_start = *it_pitchmarks - it_base_wav->fact.dwPitchLeft;
    long win_end = *it_pitchmarks + it_base_wav->fact.dwPitchRight;
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

void BaseWavsOverlapper::outputWav(string output)
{
  if (nak::compressor) {
    cout << endl << "compressing..." << endl << endl;
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
    cout << endl << "finish compressing" << endl << endl;
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

void BaseWavsOverlapper::outputWav(string output, unsigned long ms_margin)
{
  output_wav.insert(output_wav.begin(), nak::ms2pos(ms_margin, format), 0);
  outputWav(output);
}

vector<unsigned long>::iterator BaseWavsOverlapper::pos2it(unsigned long pos)
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
WavFormat BaseWavsOverlapper::getWavFormat()
{
  return format;
}

list<unsigned long> BaseWavsOverlapper::getPitchmarksList()
{
  list<unsigned long> pitchmarks(this->pitchmarks.begin(), this->pitchmarks.end());

  return pitchmarks;
}

vector<unsigned long> BaseWavsOverlapper::getPitchmarksVector()
{
  return pitchmarks;
}
