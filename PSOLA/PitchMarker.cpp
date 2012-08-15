#include "PitchMarker.h"

#pragma comment(lib, "libfftw3-3.lib")

#define _USE_MATH_DEFINES

using namespace std;

PitchMarker::PitchMarker(){}

PitchMarker::PitchMarker(short win_size)
{
  setWinSize(win_size);
}

PitchMarker::PitchMarker(short pitch, unsigned long fs)
{
  setPitch(pitch, fs);
}

PitchMarker::~PitchMarker(){}

short PitchMarker::getPitch()
{
  return this->pitch;
}

void PitchMarker::setPitch(short pitch, unsigned long fs)
{
  this->pitch = pitch;
  this->win_size = (short)(fs / pitch);
}

short PitchMarker::getWinSize()
{
  return this->win_size;
}

void PitchMarker::setWinSize(short win_size)
{
  this->pitch = 0;
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

bool PitchMarker::mark(WavData input)
{
  return mark(input.getDataVector());
}

bool PitchMarker::mark(list<short> input)
{
  vector<short> input_vector(input.size(), 0);

  long i = -1;
  for (list<short>::iterator it=input.begin(); it!=input.end(); ++it)
    input_vector[++i] = *it;

  return mark(input_vector);
}

bool PitchMarker::mark(vector<short> input)
{
  if (win_size <= 0)
    return false;
  cout << "----- start pitch marking -----" << endl;

  vector<short>::iterator mark_next, mark_prev;
  bool to_prev, to_next;
  mark_list.clear();

  // find base mark
  vector<short>::iterator it_max = max_element(input.begin()+win_size+1, input.end()-win_size-1);
  vector<short>::iterator it_min = min_element(input.begin()+win_size+1, input.end()-win_size-1);
  vector<short>::iterator it_start = (*it_max>-*it_min)?it_max:it_min;
  mark_list.push_back((mark_prev=mark_next=it_start)-input.begin());
  to_prev = to_next = true;
  cout << "win_size:" << win_size << ", start:" << mark_prev-input.begin() << ", input.size:" << input.size() << endl;

  // pitch marking
  while (to_prev || to_next) {
    // next
    if (!to_next || input.end()-mark_next<win_size) {
      to_next = false;
  	} else {
      vector<float> xcorr_win = xcorr(mark_next-win_size, mark_next+win_size-1);
      vector<float>::iterator it_max =
        max_element(xcorr_win.begin()+(win_size*2.75), xcorr_win.begin()+(win_size*3.25));

      long dist = it_max - xcorr_win.begin() - (win_size*2);
	    if (input.end()-mark_next <= dist)
	      to_next = false;
  	  else
	      mark_list.push_back((mark_next+=dist)-input.begin());
    }
    // prev
	  if (!to_prev || mark_prev-input.begin()<win_size) {
      to_prev = false;
    } else {
      vector<float> xcorr_win = xcorr(mark_prev+win_size+1, mark_prev-win_size);
      vector<float>::iterator it_max =
        max_element(xcorr_win.begin()+(win_size*2.75), xcorr_win.begin()+(win_size*3.25));

      long dist = it_max - xcorr_win.begin() - (win_size*2);
	    if (mark_prev-input.begin() <= dist)
        to_prev = false;
	    else
        mark_list.push_front((mark_prev-=dist)-input.begin());
	  }
  }

  // add first and last point
  if (mark_list.front() != 0)
    mark_list.push_front(0);
  if (mark_list.back() != input.size()-1)
    mark_list.push_back(input.size()-1);

  cout << "----- finish pitch marking -----" << endl << endl;
  return true;
}

vector<float> PitchMarker::xcorr(vector<short>::iterator first, vector<short>::iterator last)
{
  vector<short> wavdata;
  if (last-first < 0) {
    wavdata.resize(first-last);
    reverse_copy(last, first, wavdata.begin());
  } else {
    wavdata.resize(last-first);
    copy(first, last, wavdata.begin());
  }

  long fftlen = wavdata.size() * 2 - 1;
  fftw_complex *in1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));

  for (int i=0; i<fftlen; i++)
    in1[i][0] = in1[i][1] = in2[i][0] = in2[i][1] = 0;
  for (int i=0; i<wavdata.size(); i++)
    in1[i][0] = in2[i+wavdata.size()][0] = wavdata[i];

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

  vector<float> output(fftlen, 0);
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
