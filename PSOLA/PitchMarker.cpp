#include "PitchMarker.h"

#pragma comment(lib, "libfftw3-3.lib")

using namespace std;

PitchMarker::PitchMarker():pos_offs(0),pos_cons(0),pos_blnk(0){}

PitchMarker::PitchMarker(short win_size):pos_offs(0),pos_cons(0),pos_blnk(0)
{
  setWinSize(win_size);
}

PitchMarker::PitchMarker(short pitch, unsigned long fs):pos_offs(0),pos_cons(0),pos_blnk(0)
{
  setWinSize(pitch, fs);
}

PitchMarker::~PitchMarker(){}

void PitchMarker::setWinSize(double pitch, unsigned long fs)
{
  setWinSize((short)(fs / pitch));
}

short PitchMarker::getWinSize()
{
  return this->win_size;
}

void PitchMarker::setWinSize(short win_size)
{
  this->win_size = win_size;
}

list<long> PitchMarker::getMarkList()
{
  return this->mark_list;
}

vector<long> PitchMarker::getMarkVector()
{
  vector<long> mark_vector(mark_list.size());

  int i = -1;
  for (list<long>::iterator it=mark_list.begin(); it!=mark_list.end(); ++it)
    mark_vector[++i] = *it;

  return mark_vector;
}

vector<short> PitchMarker::getTargetWav()
{
  return target_wav;
}

void PitchMarker::setTargetWav(std::vector<short> target_wav)
{
  this->target_wav = target_wav;
}

long PitchMarker::getPosOffs()
{
  return pos_offs;
}

long PitchMarker::getPosCons()
{
  return pos_cons;
}

long PitchMarker::getPosBlnk()
{
  return pos_blnk;
}
/*
void PitchMarker::setConsPos(unsigned short cons, unsigned long fs)
{
  this->cons_pos = fs / 1000.0 * cons;
}
*/
void PitchMarker::setRange(unsigned short offs, unsigned short cons, unsigned short blnk, unsigned long fs)
{
  this->pos_offs = fs / 1000.0 * offs;
  this->pos_cons = fs / 1000.0 * (offs+cons);
  this->pos_blnk = fs / 1000.0 * blnk;
}

bool PitchMarker::mark(WavData input)
{
  return mark(input.getDataVector());
}

bool PitchMarker::mark(list<short> input)
{
  vector<short> input_vector(input.begin(), input.end());
  return mark(input_vector);
}

bool PitchMarker::mark(vector<short> input)
{
  if (win_size <= 0)
    return false;
  long pos_blnk = (this->pos_blnk>0)?(input.size()-this->pos_blnk):(pos_offs-this->pos_blnk);

  cout << "----- start pitch marking -----" << endl;

  // find base mark
  vector<short>::iterator it_start;
  if (pos_cons < win_size) {
    it_start = max_element(input.begin()+win_size, input.end()-win_size);
  } else {
    it_start = max_element(input.begin()+pos_cons, input.begin()+pos_cons+win_size);
  }
  vector<short>::reverse_iterator rit_start(it_start);

  vector<short>::iterator mark_next = it_start;
  vector<short>::reverse_iterator mark_prev = rit_start;

  mark_list.clear();
  mark_list.push_back((it_start)-input.begin());

  cout << "win_size:" << win_size << ", start:" << it_start-input.begin() << ", input.size:" << input.size() << endl;

  // pitch marking (foward)
  while (input.end()-mark_next>win_size*1.5 && mark_next-input.begin()<pos_blnk) {
    vector<double> xcorr_win = xcorr(it_start, mark_next, win_size);
    unsigned short pitch_margin = min((unsigned short)(xcorr_win.size()/2), nak::pitch_margin);
    long dist = max_element(xcorr_win.begin()+(xcorr_win.size()/2)-pitch_margin,
      xcorr_win.begin()+(xcorr_win.size()/2)+pitch_margin)-xcorr_win.begin();
    mark_list.push_back((mark_next+=dist)-input.begin());
  }

  // pitch marking (back)
  while (input.rend()-mark_prev>win_size*1.5 && input.rend()-mark_prev>pos_offs) {
    vector<double> xcorr_win = xcorr(rit_start, mark_prev, win_size);
    unsigned short pitch_margin = min((unsigned short)(xcorr_win.size()/2), nak::pitch_margin);
    long dist = max_element(xcorr_win.begin()+(xcorr_win.size()/2)-pitch_margin,
      xcorr_win.begin()+(xcorr_win.size()/2)+pitch_margin)-xcorr_win.begin();
    mark_list.push_front(input.rend()-(mark_prev+=dist));
  }

  cout << "----- finish pitch marking -----" << endl << endl;

  return true;
}

// cross correlation
template<typename it>
vector<double> PitchMarker::xcorr(it it_start, it it_base, short win_size)
{
  vector<short> target_wav(win_size, 0);
  for (int i=0; i<win_size; i++) {
    target_wav[i] = *(it_base+(win_size/2)+i);
  }

  return xcorr(it_start, target_wav, win_size);
}

template<typename it>
vector<double> PitchMarker::xcorr(it it_start, vector<short> target_wav, short win_size)
{
  int fftlen = win_size * 2;
  vector<double> filter = nak::getHann(win_size);

  fftw_complex *in1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));

  int pos_target_wav_start = (target_wav.size()-win_size)/2;
  for (int i=0; i<fftlen; i++)
    in1[i][0] = in1[i][1] = in2[i][0] = in2[i][1] = 0;
  for (int i=0; i<win_size; i++) {
    in1[i][0] = (*(it_start-(win_size/2)+i)) * filter[i];
    if (pos_target_wav_start+i>=0 && pos_target_wav_start+i<win_size)
      in2[i+win_size][0] = target_wav[pos_target_wav_start+i] * filter[i];
  }

  fftw_plan p1 = fftw_plan_dft_1d(fftlen, in1, out1, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p1);
  fftw_destroy_plan(p1);

  fftw_plan p2 = fftw_plan_dft_1d(fftlen, in2, out2, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p2);
  fftw_destroy_plan(p2);

  for (int i=0; i<fftlen; i++) {
    in3[i][0] = (out1[i][0]*out2[i][0])+(out1[i][1]*out2[i][1]);
    in3[i][1] = (out1[i][0]*out2[i][1])-(out1[i][1]*out2[i][0]);
  }

  fftw_plan p3 = fftw_plan_dft_1d(fftlen, in3, out3, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_execute(p3);
  fftw_destroy_plan(p3);

  vector<double> output(fftlen, 0);
  for (int i=0; i<fftlen; i++)
    output[i] = out3[i][0];

  fftw_free(in1);
  fftw_free(in2);
  fftw_free(in3);
  fftw_free(out1);
  fftw_free(out2);
  fftw_free(out3);

  return output;
}

void PitchMarker::debug(string output)
{
  if (mark_list.empty())
    return;

  ofstream ofs(output.c_str());
  for (list<long>::iterator it=mark_list.begin(); it!=mark_list.end(); ++it){
    ofs << setw(8) << *it << endl;
  }
}
