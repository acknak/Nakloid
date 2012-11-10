#include "PitchMarker.h"

#pragma comment(lib, "libfftw3-3.lib")

using namespace std;

PitchMarker::PitchMarker():pitch(0),cons_pos(0){}

PitchMarker::PitchMarker(short win_size):pitch(0),cons_pos(0)
{
  setWinSize(win_size);
}

PitchMarker::PitchMarker(short pitch, unsigned long fs):cons_pos(0)
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
  setWinSize((short)(fs / pitch));
}

short PitchMarker::getWinSize()
{
  return this->win_size;
}

void PitchMarker::setWinSize(short win_size)
{
  this->win_size = win_size;
  filter = getHann(win_size);
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

long PitchMarker::getConsPos()
{
  return cons_pos;
}

void PitchMarker::setConsPos(unsigned short cons, unsigned long fs)
{
  this->cons_pos = fs / 1000.0 * cons;
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
  mark_list.clear();

  // find base mark
  vector<short>::iterator it_start;
  if (cons_pos == 0) {
    vector<short>::iterator it_max = max_element(input.begin()+win_size+1, input.end()-win_size-1);
    vector<short>::iterator it_min = min_element(input.begin()+win_size+1, input.end()-win_size-1);
    it_start = (*it_max>-*it_min)?it_max:it_min;
  } else {
    vector<short>::iterator it_max = max_element(input.begin()+cons_pos, input.begin()+cons_pos+win_size);
    vector<short>::iterator it_min = min_element(input.begin()+cons_pos, input.begin()+cons_pos+win_size);
    it_start = (*it_max>-*it_min)?it_max:it_min;
  }
  mark_list.push_back((mark_prev=mark_next=it_start)-input.begin());
  cout << "win_size:" << win_size << ", start:" << mark_prev-input.begin() << ", input.size:" << input.size() << endl;

  // pitch marking
  while (input.end()-mark_next > win_size*1.5) {
    vector<float> xcorr_win = xcorr(it_start, mark_next, win_size);
    long dist = max_element(xcorr_win.begin(),xcorr_win.end())-xcorr_win.begin();
    mark_list.push_back((mark_next+=dist)-input.begin());
  }
  if (input.end()-mark_next > win_size)
    mark_list.push_back(mark_next+win_size-input.begin());
  mark_list.push_back(input.size()-1);
  /*
  while (mark_prev-input.begin() > win_size*1.5) {
    vector<float> xcorr_win = xcorr(mark_prev, it_start, -win_size);
    long dist = max_element(xcorr_win.begin(),xcorr_win.end())-xcorr_win.begin();
    mark_list.push_front((mark_prev-=dist)-input.begin());
  }
  if (mark_prev-input.begin() > win_size)
    mark_list.push_front(mark_prev-win_size-input.begin());
  mark_list.push_front(0);
  */
  short dist = *(++mark_list.begin())-mark_list.front();
  while (mark_prev-input.begin() > max(dist,win_size)*1.5) {
    vector<float> xcorr_win = xcorr(mark_prev+dist, mark_prev, -win_size);
    dist = max_element(xcorr_win.begin(),xcorr_win.end())-xcorr_win.begin();
    mark_list.push_front((mark_prev-=dist)-input.begin());
    /*
    vector<float> xcorr_win = xcorr(mark_prev+tmp_win_size, mark_prev-tmp_win_size);
    vector<float>::iterator it_max =
      max_element(xcorr_win.begin()+(tmp_win_size*2.75), xcorr_win.begin()+(tmp_win_size*3.25));
    long dist = it_max - xcorr_win.begin() - (tmp_win_size*2);
	  if (mark_prev-input.begin() > dist)
      mark_list.push_front((mark_prev-=dist)-input.begin());
    else
      break;
    */
  }
  if (mark_prev-input.begin() > max(dist,win_size))
    mark_list.push_front(max(dist,win_size));
  mark_list.push_front(0);

  cout << "----- finish pitch marking -----" << endl << endl;
  return true;
}

// cross correlation
vector<float> PitchMarker::xcorr(vector<short>::iterator it_start, vector<short>::iterator it_base, short exp_dist)
{
  long win_size=exp_dist*((exp_dist>0)?1:-1), fftlen=win_size*2;

  fftw_complex *in1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));

  if (exp_dist > 0) {
    for (int i=0; i<win_size; i++) {
      in1[i][0] = (*(it_start-(win_size/2)+i)) * filter[i];
      in2[i+win_size][0] = *(it_base+(win_size/2)+i) * filter[i];
    }
  } else {
    vector<double> tmp_filter = getHann(win_size);
    for (int i=0; i<win_size; i++) {
      in1[i][0] = *(it_start+(win_size/2)-i) * tmp_filter[i];
      in2[i+win_size][0] = *(it_base-(win_size/2)-i) * tmp_filter[i];
    }
  }
  for (int i=0; i<fftlen; i++)
    in1[i][1] = in2[i][1] = 0;

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

vector<double> PitchMarker::getHann(long len)
{
  vector<double> filter(len, 0);

  for (int i=0; i<filter.size(); ++i) {
    double x = (i+1.0) / (filter.size()+1.0);
    filter[i] = 0.5 - (0.5 * cos(2*M_PI*x));
  }

  return filter;
}
