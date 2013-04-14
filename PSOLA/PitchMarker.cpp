#include "PitchMarker.h"

#pragma comment(lib, "libfftw3-3.lib")

using namespace std;

PitchMarker::PitchMarker(){}

PitchMarker::~PitchMarker(){}

bool PitchMarker::mark(vector<short> vowel_wav)
{
  if (input_wav.size()==0) {
    cerr << "[PitchMarker::mark] input_wav is invalid" << endl;
    return false;
  }
  if (vowel_wav.size()==0) {
    cerr << "[PitchMarker::mark] vowel_wav is invalid" << endl;
    return false;
  }
  this->vowel_wav = vowel_wav;
  short win_size = vowel_wav.size();
  mark_list.clear();

  // pitch marking (foward)
  vector<short>::iterator mark_next = max_element(it_input_wav_cons, it_input_wav_cons+win_size);
  while (mark_next<it_input_wav_blnk && input_wav.end()-mark_next>win_size*1.5) {
    vector<double> xcorr_win = xcorr(mark_next, false);
    unsigned short pitch_margin = min((unsigned short)(xcorr_win.size()/2), nak::pitch_margin);
    long dist = max_element(xcorr_win.begin()+(xcorr_win.size()/2)-pitch_margin,
      xcorr_win.begin()+(xcorr_win.size()/2)+pitch_margin)-xcorr_win.begin();
    mark_list.push_back((mark_next+=dist)-input_wav.begin());
  }
  if (mark_list.size()==0) {
    cerr << "[PitchMarker::mark] pitch marking (forward) failed" << endl;
    return false;
  }

  // pitch marking (back)
  vector<short>::iterator mark_prev(input_wav.begin()+mark_list.front());
  while (mark_prev>it_input_wav_offs && mark_prev-input_wav.begin()>win_size*1.5) {
    vector<double> xcorr_win = xcorr(mark_prev, true);
    unsigned short pitch_margin = min((unsigned short)(xcorr_win.size()/2), nak::pitch_margin);
    long dist = max_element(xcorr_win.begin()+(xcorr_win.size()/2)-pitch_margin,
      xcorr_win.begin()+(xcorr_win.size()/2)+pitch_margin)-xcorr_win.begin();
    mark_list.push_front((mark_prev-=dist)-input_wav.begin());
  }

  return true;
}

bool PitchMarker::mark(double hz, long fs)
{
  short win_size = fs / hz;
  vector<short>::iterator it_input_wav_max = max_element(it_input_wav_cons, it_input_wav_cons+win_size);
  vector<short> vowel_wav(it_input_wav_max-(win_size/2), it_input_wav_max-(win_size/2)+win_size);
  return mark(vowel_wav);
}

void PitchMarker::setInputWav(vector<short>input_wav)
{
  this->input_wav = input_wav;
  this->pos_offs = 0;
  this->it_input_wav_offs = this->input_wav.begin();
  vector<short>::iterator it_input_wav_max = max_element(this->input_wav.begin(), this->input_wav.end());
  this->it_input_wav_blnk = this->input_wav.end();
}

void PitchMarker::setInputWav(vector<short>input_wav, short ms_offs, short ms_cons, short ms_blnk, unsigned long fs)
{
  long pos_offs=fs/1000.0*ms_offs, pos_cons=fs/1000.0*ms_cons, pos_blnk=fs/1000.0*ms_blnk;
  pos_blnk = (pos_blnk>0)?(input_wav.size()-pos_blnk):(pos_offs-pos_blnk);
  this->input_wav = input_wav;
  this->it_input_wav_offs = this->input_wav.begin() + pos_offs;
  this->it_input_wav_cons = this->input_wav.begin() + pos_offs + pos_cons;
  this->it_input_wav_blnk = this->input_wav.begin() + pos_blnk;
}

list<long> PitchMarker::getMarkList()
{
  return this->mark_list;
}

vector<long> PitchMarker::getMarkVector()
{
  vector<long> mark_vector(mark_list.begin(), mark_list.end());
  return mark_vector;
}

// cross correlation
vector<double> PitchMarker::xcorr(vector<short>::iterator it_target, bool reverse)
{
  short win_size = vowel_wav.size();
  int fftlen = win_size * 2;
  vector<double> filter = nak::getHann(win_size);

  fftw_complex *in1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));

  for (int i=0; i<fftlen; i++)
    in1[i][0] = in1[i][1] = in2[i][0] = in2[i][1] = 0;
  if (reverse) {
    for (int i=0; i<win_size; i++) {
      in1[i][0] = *(vowel_wav.rbegin()+i) * filter[i];
      in2[i+win_size][0] = *(it_target+(win_size/2)-i) * filter[i];
    }
  } else {
    for (int i=0; i<win_size; i++) {
      in1[i][0] = *(vowel_wav.begin()+i) * filter[i];
      in2[i+win_size][0] = *(it_target-(win_size/2)+i) * filter[i];
    }
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
