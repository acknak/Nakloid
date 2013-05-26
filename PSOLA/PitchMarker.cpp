#include "PitchMarker.h"

#pragma comment(lib, "libfftw3-3.lib")

using namespace std;

PitchMarker::PitchMarker(){}

PitchMarker::~PitchMarker(){}

bool PitchMarker::mark(vector<short> fore_vowel_wav, vector<short> aft_vowel_wav)
{
  if (input_wav.size()==0) {
    cerr << "[PitchMarker::mark] input_wav is invalid" << endl;
    return false;
  }
  if (fore_vowel_wav.size()==0 || aft_vowel_wav.size()==0) {
    cerr << "[PitchMarker::mark] vowel_wav is invalid" << endl;
    return false;
  }

  if (fore_vowel_wav.size() > aft_vowel_wav.size()) {
    short space = fore_vowel_wav.size() - aft_vowel_wav.size();
    aft_vowel_wav.insert(aft_vowel_wav.begin(), space/2, 0);
    aft_vowel_wav.insert(aft_vowel_wav.end(), space-(space/2), 0);
  } else if (fore_vowel_wav.size() > aft_vowel_wav.size()) {
    short space = aft_vowel_wav.size() - fore_vowel_wav.size();
    fore_vowel_wav.insert(fore_vowel_wav.begin(), space/2, 0);
    fore_vowel_wav.insert(fore_vowel_wav.end(), space-(space/2), 0);
  }

  short win_size = fore_vowel_wav.size();
  pitchmarks.clear();
  pitchmarks.reserve((it_input_wav_blnk-it_input_wav_offs)/win_size);

  vector<short>::iterator it_mark_start;
  vector<short>::reverse_iterator rit_mark_start;
  vector<double> xcorr_win(win_size*2, 0.0);

  // pitch marking
  {
    // fore vowel & consonant
    it_mark_start = max_element(it_input_wav_offs+(win_size/2), it_input_wav_offs+(win_size/2*3));
    xcorr(fore_vowel_wav.begin(), fore_vowel_wav.end(), it_mark_start, xcorr_win.begin());
    it_mark_start += max_element(xcorr_win.begin(), xcorr_win.end()) - xcorr_win.begin() - (win_size/2);

    vector<vector<short>::iterator> tmp_pitchmarks_fore_vowel =
      mark(fore_vowel_wav.begin(), fore_vowel_wav.end(),
      it_mark_start, it_input_wav_ovrl, input_wav.end(), false);
    for(int i=0; i<tmp_pitchmarks_fore_vowel.size(); i++) {
      pitchmarks.push_back(tmp_pitchmarks_fore_vowel[i] - input_wav.begin());
    }

    vector<vector<short>::iterator> tmp_pitchmarks_consonant =
      mark(*(tmp_pitchmarks_fore_vowel.end()-3), tmp_pitchmarks_fore_vowel.back(),
      tmp_pitchmarks_fore_vowel.back(), it_input_wav_prec, input_wav.end(), true);
    for(int i=0; i<tmp_pitchmarks_consonant.size(); i++) {
      pitchmarks.push_back(tmp_pitchmarks_consonant[i] - input_wav.begin());
    }

    // aft vowel
    vector<vector<short>::iterator> tmp_pitchmarks_aft_vowel =
      mark(aft_vowel_wav.begin(), aft_vowel_wav.end(),
      tmp_pitchmarks_consonant.back(), it_input_wav_blnk, input_wav.end(), false);
    for(int i=0; i<tmp_pitchmarks_aft_vowel.size(); i++) {
      pitchmarks.push_back(tmp_pitchmarks_aft_vowel[i] - input_wav.begin());
    }
  }

  sort(pitchmarks.begin(), pitchmarks.end());
  pitchmarks.erase(unique(pitchmarks.begin(), pitchmarks.end()), pitchmarks.end());

  return true;
}

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

  short win_size = vowel_wav.size();
  pitchmarks.clear();
  pitchmarks.reserve((it_input_wav_blnk-it_input_wav_offs)/win_size);

  vector<short>::reverse_iterator rit_input_wav_offs(it_input_wav_blnk);
  vector<short>::reverse_iterator rit_input_wav_prec(it_input_wav_prec);
  vector<short>::reverse_iterator rit_input_wav_blnk(it_input_wav_offs);
  vector<short>::reverse_iterator rit_mark_start;

  // find start point
  {
    rit_mark_start = max_element(rit_input_wav_offs+(win_size/2), rit_input_wav_offs+(win_size/2*3));
    vector<double> xcorr_win(win_size*2, 0.0);
    xcorr(vowel_wav.rbegin(), vowel_wav.rend(), rit_mark_start, xcorr_win.begin());
    long dist = max_element(xcorr_win.begin(), xcorr_win.end())-xcorr_win.begin();
    rit_mark_start += dist - (win_size/2);
  }

  // pitch marking
  {
    vector<vector<short>::reverse_iterator> tmp_pitchmarks_vowel =
      mark(vowel_wav.rbegin(), vowel_wav.rend(),
      rit_mark_start, rit_input_wav_prec, input_wav.rend(), false);
    for(int i=0; i<tmp_pitchmarks_vowel.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks_vowel[i]);
    }
    vector<vector<short>::reverse_iterator> tmp_pitchmarks_consonant =
      mark(*(tmp_pitchmarks_vowel.end()-3), tmp_pitchmarks_vowel.back(),
      tmp_pitchmarks_vowel.back(), rit_input_wav_blnk, input_wav.rend(), true);
    for(int i=0; i<tmp_pitchmarks_consonant.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks_consonant[i]);
    }
  }

  sort(pitchmarks.begin(), pitchmarks.end());
  pitchmarks.erase(unique(pitchmarks.begin(), pitchmarks.end()), pitchmarks.end());

  return true;
}

bool PitchMarker::mark(double hz, long fs)
{
  short win_size = fs / hz;
  vector<short>::iterator it_input_wav_max = max_element(it_input_wav_prec, it_input_wav_prec+win_size);
  vector<short> vowel_wav(it_input_wav_max-(win_size/2), it_input_wav_max-(win_size/2)+win_size);
  return mark(vowel_wav);
}

template <class Iterator>
vector<Iterator> PitchMarker::mark(Iterator it_vowel_begin, Iterator it_vowel_end,
                                   Iterator it_target_begin, Iterator it_target_end, Iterator it_wav_end, bool autocorrelation)
{
  short win_size = it_vowel_end-it_vowel_begin;
  Iterator tmp_pitchmark = it_target_begin;
  vector<double> xcorr_win(win_size*2, 0.0);
  vector<Iterator> pitchmarks(1, it_target_begin);
  pitchmarks.reserve((it_target_end-it_target_begin)/win_size);

  long dist = win_size/2;
  while (tmp_pitchmark<it_target_end && it_wav_end-tmp_pitchmark>win_size) {
    if (autocorrelation && pitchmarks.size()>1) {
      xcorr(pitchmarks.back()-(win_size/2), pitchmarks.back()+(win_size/2), tmp_pitchmark+(win_size/2), xcorr_win.begin());
    } else {
      xcorr(it_vowel_begin, it_vowel_end, tmp_pitchmark+(win_size/2), xcorr_win.begin());
    }
    short margin_left, margin_right;
    if (dist>win_size/2) {
      margin_left = win_size-nak::pitch_margin;
      margin_right = (dist*2)+nak::pitch_margin;
    } else {
      margin_left = (dist*2)-nak::pitch_margin;
      margin_right = win_size+nak::pitch_margin;
    }
    if (margin_left <= (win_size/4*3)) {
      margin_left = (win_size/4*3)+1;
    } else if (margin_right >= xcorr_win.size()) {
      margin_right = xcorr_win.size()-1;
    }
    dist = max_element(xcorr_win.begin()+margin_left, xcorr_win.begin()+margin_right) - xcorr_win.begin() - (win_size/2);
    pitchmarks.push_back(tmp_pitchmark+=dist);
  }

  return pitchmarks;
}

void PitchMarker::setInputWav(vector<short>input_wav)
{
  this->input_wav = input_wav;
  this->pos_offs = 0;
  this->it_input_wav_offs = this->input_wav.begin();
  this->it_input_wav_blnk = this->input_wav.end();
}

void PitchMarker::setInputWav(vector<short>input_wav, short ms_offs, short ms_ovrl, short ms_prec, short ms_blnk, unsigned long fs)
{
  this->input_wav = input_wav;
  this->it_input_wav_offs = this->input_wav.begin() + (fs/1000.0*ms_offs);
  this->it_input_wav_ovrl = this->it_input_wav_offs + (fs/1000.0*ms_ovrl);
  this->it_input_wav_prec = this->it_input_wav_offs + (fs/1000.0*ms_prec);

  long pos_blnk = fs / 1000.0 * ms_blnk;
  if (pos_blnk > 0) {
    this->it_input_wav_blnk = this->input_wav.end() - pos_blnk;
  } else {
    this->it_input_wav_blnk = this->it_input_wav_offs - pos_blnk;
  }
}

vector<long> PitchMarker::getPitchMarks()
{
  return pitchmarks;
}

template <class Iterator>
void PitchMarker::xcorr(Iterator it_vowel_begin, Iterator it_vowel_end,
                        Iterator it_target_begin, vector<double>::iterator it_output)
{
  short win_size = it_vowel_end - it_vowel_begin;
  int fftlen = win_size * 2;
  vector<double> filter = nak::getHann(win_size);

  fftw_complex *in1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));

  for (int i=0; i<fftlen; i++) {
    in1[i][0] = in1[i][1] = in2[i][0] = in2[i][1] = 0;
  }
  for (int i=0; i<win_size; i++) {
    in1[i][0] = *(it_vowel_begin+i) * filter[i];
    in2[i+win_size][0] = *(it_target_begin-(win_size/2)+i) * filter[i];
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

  for (int i=0; i<fftlen; i++) {
    *(it_output+i) = out3[i][0];
  }

  fftw_free(in1);
  fftw_free(in2);
  fftw_free(in3);
  fftw_free(out1);
  fftw_free(out2);
  fftw_free(out3);
}
