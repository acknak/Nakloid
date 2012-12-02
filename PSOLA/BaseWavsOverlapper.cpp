#include "BaseWavsOverlapper.h"

using namespace std;

BaseWavsOverlapper::BaseWavsOverlapper(WavFormat format, list<double> pitches)
{
  vector<double> pitches_vector(pitches.begin(), pitches.end());
  BaseWavsOverlapper(format, pitches_vector);
}

BaseWavsOverlapper::BaseWavsOverlapper(WavFormat format, vector<double> pitches)
{
  this->format = format;
  unsigned long tmp_ms = 0;
  list<unsigned long> tmp_pitchmarks(0);
  bool is_note_on = false;

  while (tmp_ms < pitches.size()) {
    if (pitches[tmp_ms] > 0) {
      if (!is_note_on)
        tmp_pitchmarks.push_back(ms2pos(tmp_ms, format));
      tmp_pitchmarks.push_back(tmp_pitchmarks.back()+(1.0/pitches[tmp_ms]*format.dwSamplesPerSec));
      tmp_ms = pos2ms(tmp_pitchmarks.back(), format);
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

bool BaseWavsOverlapper::overlapping(unsigned long ms_start, unsigned long ms_end, BaseWavsContainer bwc, vector<unsigned char> velocities)
{
  if (pitchmarks.empty())
    return false;
  if (ms_start >= ms_end)
    return false;
  if (bwc.base_wavs.empty())
    return false;
  cout << "----- start overlapping -----" << endl;

  unsigned long fade_start = (bwc.base_wavs.end()-1-((bwc.base_wavs.size()-1-bwc.format.dwRepeatStart)/2))->fact.dwPosition;
  unsigned long fade_last = bwc.base_wavs.back().fact.dwPosition;
  vector<unsigned long>::iterator it_begin_pitchmarks = pos2it(ms2pos(ms_start,format));
  vector<unsigned long>::iterator it_end_pitchmarks = pos2it(ms2pos(ms_end,format));
  vector<unsigned long>::iterator it_pitchmarks = it_begin_pitchmarks;
  cout << "base_wav size:" << bwc.base_wavs.size() << endl;
  cout << "fade_start:" << fade_start << ", fade_last:" << fade_last << endl;

  do {
    // choose overlap base_wav
    vector<BaseWav>::iterator it_base_wav = bwc.base_wavs.begin();
    unsigned long dist = *it_pitchmarks - *it_begin_pitchmarks;
    if (dist > fade_last)
      dist = (dist-fade_start)%(fade_last-fade_start+1) + fade_start;
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
    unsigned long ms_dist = pos2ms(*it_pitchmarks-*it_begin_pitchmarks,format);
    double scale = ((ms_dist<velocities.size())?velocities[ms_dist]:velocities.back())/100.0;
    for (int i=0; i<win_end-win_start; i++)
      output_wav[win_start+i] += win[i] * scale;
  } while (++it_pitchmarks != it_end_pitchmarks);

  cout << "----- finish overlapping -----" << endl << endl;
  return true;
}

void BaseWavsOverlapper::outputWav(string output)
{
  long size = output_wav.size()*sizeof(short);
  ofstream ofs(output.c_str(), ios_base::out|ios_base::trunc|ios_base::binary);
  WavParser::setWavHeader(&ofs, format, size+28);
  ofs.write((char*)WavFormat::data, sizeof(char)*4);
  ofs.write((char*)&size, sizeof(long));
  ofs.write((char*)(&output_wav[0]), size);
  ofs.close();
}

void BaseWavsOverlapper::debug(string output)
{
  ofstream ofs(output.c_str());

  for (int i=0; i<output_wav.size(); i++)
    ofs << output_wav[i] << endl;
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
