#include "BaseWavsOverlapper.h"

using namespace std;

BaseWavsOverlapper::BaseWavsOverlapper():rep_start(-1), velocity(1.0), is_normalize(true){}

BaseWavsOverlapper::~BaseWavsOverlapper(){}

void BaseWavsOverlapper::setPitchMarks(list<long> pitch_marks)
{
  this->pitch_marks.clear();
  this->pitch_marks.assign(pitch_marks.size(), 0);

  for (list<long>::iterator it=pitch_marks.begin(); it!=pitch_marks.end(); ++it)
    this->pitch_marks[*it];
}

void BaseWavsOverlapper::setPitchMarks(vector<long> pitch_marks)
{
  this->pitch_marks = pitch_marks;
}

list<long> BaseWavsOverlapper::getPitchMarkList()
{
  list<long> pitch_marks;

  for (int i=0; i<this->pitch_marks.size(); ++i)
    pitch_marks.push_back(this->pitch_marks[i]);

  return pitch_marks;
}

vector<long> BaseWavsOverlapper::getPitchMarkVector()
{
  return pitch_marks;
}

void BaseWavsOverlapper::setBaseWavs(vector<BaseWav> base_wavs)
{
  this->base_wavs = base_wavs;
}

vector<BaseWav> BaseWavsOverlapper::getBaseWavs()
{
  return base_wavs;
}

list<short> BaseWavsOverlapper::getOutputWavList()
{
  list<short> output_wav;

  for (int i=0; i<this->output_wav.size(); ++i)
    output_wav.push_back(this->output_wav[i]);

  return output_wav;
}

vector<short> BaseWavsOverlapper::getOutputWavVector()
{
  return output_wav;
}

void BaseWavsOverlapper::setRepStart(long rep_start)
{
  this->rep_start = rep_start;
}

long BaseWavsOverlapper::getRepStart()
{
  return rep_start;
}

void BaseWavsOverlapper::setVelocity(unsigned short velocity)
{
  this->velocity = velocity/100.0;
}

void BaseWavsOverlapper::setVelocity(double velocity)
{
  this->velocity = velocity;
}

double BaseWavsOverlapper::getVelocity()
{
  return velocity;
}

void BaseWavsOverlapper::isNormalize(bool is_normalize)
{
  this->is_normalize = is_normalize;
}

bool BaseWavsOverlapper::isNormalize()
{
  return is_normalize;
}

bool BaseWavsOverlapper::overlapping()
{
  if (pitch_marks.empty() || base_wavs.empty() || rep_start<0)
    return false;
  cout << "----- start overlapping -----" << endl;

  output_wav.clear();
  output_wav.assign(pitch_marks.back(), 0);
  long morph_start = base_wavs[rep_start+((base_wavs.size()-rep_start)/2)].fact.dwPosition;
  long morph_last = base_wavs.back().fact.dwPosition;
  vector<BaseWav>::iterator tmp_base_wav = base_wavs.begin();
  cout << "output size:" << output_wav.size() << endl;
  cout << "morph_start:" << morph_start << ", morph_last:" << morph_last << endl;

  if (is_normalize) {
    double target_rms = getRMS(base_wavs[rep_start].data.getDataVector());
    for (int i=rep_start+1; i<base_wavs.size(); i++)
      base_wavs[i].data.setData(normalize(base_wavs[i].data.getDataVector(), target_rms));
  }

  for (int i=0; i<pitch_marks.size()-1; i++) {
    long tmp_dist, tmp_pitch_mark = (pitch_marks[i] > morph_last)
      ? morph_start + ((pitch_marks[i]-morph_last)%(morph_last-morph_start))
      : pitch_marks[i];
    long num_base_wav = 0;
    do {
      tmp_dist = abs(tmp_pitch_mark-(base_wavs[num_base_wav].fact.dwPosition));
      if (++num_base_wav == base_wavs.size()) {
        --num_base_wav;
        break;
      }
    } while (tmp_dist>abs(tmp_pitch_mark-(base_wavs[num_base_wav]).fact.dwPosition));
    BaseWav base_wav = base_wavs[--num_base_wav];
    vector<short> win = base_wav.data.getDataVector();
    long win_start = pitch_marks[i] - base_wav.fact.dwPitchLeft;
    long win_end = pitch_marks[i] + base_wav.fact.dwPitchRight;
    if (win_start < 0) {
      // left edge
      win.erase(win.begin(), win.begin()-win_start);
      win_start = 0;
    }
    if (win_end >= pitch_marks.back()) {
      // right edge
      win.erase(win.end()-(win_end-pitch_marks.back()), win.end());
      win_end = pitch_marks.back();
    }
    for (int j=0; j<win_end-win_start; j++)
      output_wav[win_start+j] += win[j];
  }

  if (velocity != 1.0)
    for (int i=0; i<output_wav.size(); i++)
      output_wav[i] *= velocity;

  cout << "----- finish overlapping -----" << endl << endl;
  return true;
}

void BaseWavsOverlapper::debugTxt(string output)
{
  ofstream ofs(output.c_str());

  for (int i=0; i<output_wav.size(); i++)
    ofs << output_wav[i] << endl;
}

double BaseWavsOverlapper::getRMS(vector<short> wav)
{
  double rms = 0.0;
  for (int i=0; i<wav.size(); i++)
    rms += pow((double)wav[i], 2) / wav.size();
  return sqrt(rms);
}

vector<short> BaseWavsOverlapper::normalize(vector<short> wav, double target_rms)
{
  double wav_rms = getRMS(wav);
  for (int i=0; i<wav.size(); i++)
    wav[i] = wav[i] * (target_rms/wav_rms);

  return wav;
}

void BaseWavsOverlapper::debugWav(string output)
{
  cout << "start wav output" << endl;

  long size_all = 28 + (output_wav.size()*sizeof(short)+8);
  WavFormat format;
  format.setDefaultValues();
  ofstream ofs(output.c_str(), ios_base::out|ios_base::trunc|ios_base::binary);
  WavParser::setWavHeader(&ofs, format, size_all);

  // data chunk
  WavData data;
  data.setData(output_wav);
  long data_size = data.getSize(); 
  ofs.write((char*)WavFormat::data, sizeof(char)*4);
  ofs.write((char*)&data_size, sizeof(long));
  ofs.write((char*)data.getData(), data.getSize());

  cout << "finish wav output" << endl;
}
