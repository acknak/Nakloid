#include "PitchMarker.h"

#pragma comment(lib, "fftss.lib")
#include "core-inl.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <fftw3.h>

using namespace std;

struct PitchMarker::Parameters PitchMarker::params;

PitchMarker::PitchMarker(const vector<double>& input_wav, short ms_offs, short ms_ovrl, short ms_prec, short ms_blnk, unsigned long fs)
  :input_wav(input_wav)
{
  setInputWavParam(ms_offs, ms_ovrl, ms_prec, ms_blnk, fs);
}

bool PitchMarker::mark(const vector<double>& fore_vowel_wav, const vector<double>& aft_vowel_wav)
{
  if (input_wav.size()==0) {
    cerr << "[PitchMarker::mark] input_wav is invalid" << endl;
    return false;
  }
  if (fore_vowel_wav.size()==0 || aft_vowel_wav.size()==0 || fore_vowel_wav.size()!=aft_vowel_wav.size()) {
    cerr << "[PitchMarker::mark] vowel_wav is invalid" << endl;
    return false;
  }

  short win_size = fore_vowel_wav.size();
  pitchmarks.clear();
  pitchmarks.reserve((it_input_wav_blnk-it_input_wav_offs)/win_size);

  long pos_cons_start, pitch_cons_start, pos_cons_end, pitch_cons_end, pos_fade_start=0, pos_fade_end=0;
  {
    vector<double>::const_iterator it_mark_start;
    {
      // find start point
      vector<double> xcorr_win(win_size*2, 0.0);
      it_mark_start = max_element(it_input_wav_offs+(win_size/2), it_input_wav_offs+(win_size/2*3));
      xcorr(it_mark_start, xcorr_win.begin(), fore_vowel_wav.begin(), fore_vowel_wav.end());
      it_mark_start += max_element(xcorr_win.begin(), xcorr_win.end()) - xcorr_win.begin() - (win_size/2);
    }
    // fore vowel pitch mark
    vector<vector<double>::const_iterator> tmp_pitchmarks =
      markWithVowel(it_mark_start, it_input_wav_ovrl, fore_vowel_wav.begin(), fore_vowel_wav.end());
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(tmp_pitchmarks[i] - input_wav.begin());
    }
    tmp_pitchmarks = markWithSelf(tmp_pitchmarks.back(), it_input_wav_prec, *(tmp_pitchmarks.end()-3), *(tmp_pitchmarks.end()-1));
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(tmp_pitchmarks[i] - input_wav.begin());
    }
    pos_cons_start = pitchmarks.back();
    pitch_cons_start = *(pitchmarks.end()-1) - *(pitchmarks.end()-2);
  }
  {
    vector<double>::const_reverse_iterator rit_mark_start, rit_input_wav_offs(it_input_wav_blnk), rit_input_wav_prec(it_input_wav_prec), rit_prec_start(input_wav.begin()+pos_cons_start);
    {
      // find start point
      vector<double> xcorr_win(win_size*2, 0.0);
      rit_mark_start = max_element(rit_input_wav_offs+(win_size/2), rit_input_wav_offs+(win_size/2*3));
      xcorr(rit_mark_start, xcorr_win.begin(), aft_vowel_wav.rbegin(), aft_vowel_wav.rend());
      rit_mark_start += max_element(xcorr_win.begin(), xcorr_win.end()) - xcorr_win.begin() - (win_size/2);
    }
    // aft vowel pitch mark
    vector<double> xcorr_std;
    vector<vector<double>::const_reverse_iterator> tmp_pitchmarks =
      markWithVowel(rit_mark_start, rit_input_wav_prec, aft_vowel_wav.rbegin(), aft_vowel_wav.rend(), &xcorr_std);
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[i]);
      if (pos_fade_start==0 && xcorr_std[xcorr_std.size()-i-1] > 0.0) {
        pos_fade_start = input_wav.rend() - tmp_pitchmarks[xcorr_std.size()-i-1];
      }
      if (pos_fade_end==0 && xcorr_std[i] > 0.0) {
        pos_fade_end = input_wav.rend() - tmp_pitchmarks[i];
      }
    }
    tmp_pitchmarks = markWithSelf(tmp_pitchmarks.back(), rit_prec_start, *(tmp_pitchmarks.end()-3), *(tmp_pitchmarks.end()-1));
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[i]);
    }
    pos_cons_end = pitchmarks.back();
    pitch_cons_end = *(pitchmarks.end()-2) - *(pitchmarks.end()-1);
  }
  {
    // consonant pitch mark
    long pos_len=pos_cons_end-pos_cons_start, pitch_avg=(pitch_cons_start+pitch_cons_end)/2;
    if (pos_len > pitch_avg) {
      long num_pitchmark = (long)((pos_len/(double)pitch_avg)+0.5) - 1;
      for (size_t i=0; i<num_pitchmark; i++) {
        pitchmarks.push_back(pos_len/(num_pitchmark+1.0)*(i+1)+pos_cons_start);
      }
    }
  }

  sort(pitchmarks.begin(), pitchmarks.end());
  pitchmarks.erase(unique(pitchmarks.begin(), pitchmarks.end()), pitchmarks.end());

  sub_fade_start = find(pitchmarks.begin(), pitchmarks.end() , pos_fade_start) - pitchmarks.begin();
  sub_fade_end = find(pitchmarks.begin(), pitchmarks.end() , pos_fade_end) - pitchmarks.begin();

  return true;
}

bool PitchMarker::mark(const vector<double>& vowel_wav)
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

  vector<double>::const_reverse_iterator rit_input_wav_offs(it_input_wav_blnk), rit_input_wav_prec(it_input_wav_prec), rit_base_start, rit_base_end;
  long pos_fade_start=0, pos_fade_end=0;
  {
    vector<double>::const_reverse_iterator rit_mark_start;
    {
      // find start point
      vector<double> xcorr_win(win_size*2, 0.0);
      rit_mark_start = max_element(rit_input_wav_offs+(win_size/2), rit_input_wav_offs+(win_size/2*3));
      xcorr(rit_mark_start, xcorr_win.begin(), vowel_wav.rbegin(), vowel_wav.rend());
      rit_mark_start += max_element(xcorr_win.begin(), xcorr_win.end()) - xcorr_win.begin() - (win_size/2);
    }
    // vowel pitch mark
    vector<double> xcorr_std;
    vector<vector<double>::const_reverse_iterator> tmp_pitchmarks =
      markWithVowel(rit_mark_start, rit_input_wav_prec, vowel_wav.rbegin(), vowel_wav.rend(), &xcorr_std);
    if (tmp_pitchmarks.size() > 0) {
      for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
        pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[i]);
        if (pos_fade_start==0 && xcorr_std[xcorr_std.size()-i-1] > 0.0) {
          pos_fade_start = input_wav.rend() - tmp_pitchmarks[xcorr_std.size()-i-1];
        }
        if (pos_fade_end==0 && xcorr_std[i] > 0.0) {
          pos_fade_end = input_wav.rend() - tmp_pitchmarks[i];
        }
      }
    } else {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[0]);
      pos_fade_start = pos_fade_end = pitchmarks.back();
    }
    if (tmp_pitchmarks.size() < 3) {
      rit_base_start = tmp_pitchmarks.back() - (win_size/2);
      rit_base_end = tmp_pitchmarks.back() + (win_size/2);
    } else {
      rit_base_start = *(tmp_pitchmarks.end()-3);
      rit_base_end = *(tmp_pitchmarks.end()-1);
    }
  }
  {
    // consonant pitch mark
    vector<double>::const_reverse_iterator rit_input_wav_blnk(it_input_wav_offs);
    vector<vector<double>::const_reverse_iterator> tmp_pitchmarks =
      markWithSelf(rit_base_end, rit_input_wav_blnk, rit_base_start, rit_base_end);
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[i]);
    }
  }

  sort(pitchmarks.begin(), pitchmarks.end());
  pitchmarks.erase(unique(pitchmarks.begin(), pitchmarks.end()), pitchmarks.end());

  sub_fade_start = find(pitchmarks.begin(), pitchmarks.end() , pos_fade_start) - pitchmarks.begin();
  sub_fade_end = find(pitchmarks.begin(), pitchmarks.end() , pos_fade_end) - pitchmarks.begin();

  return true;
}

bool PitchMarker::mark(double hz, unsigned long fs)
{
  short win_size = fs / hz;
  vector<double>::const_iterator it_input_wav_max = max_element(it_input_wav_prec, it_input_wav_prec+win_size);
  vector<double> vowel_wav(it_input_wav_max-(win_size/2), it_input_wav_max-(win_size/2)+win_size);
  return mark(vowel_wav);
}

template <class Iterator>
vector<Iterator> PitchMarker::markWithVowel(Iterator it_input_begin, Iterator it_input_end,
                                            Iterator it_vowel_begin, Iterator it_vowel_end) const
{
  return markWithVowel(it_input_begin,it_input_end, it_vowel_begin, it_vowel_end, 0);
}

template <class Iterator>
vector<Iterator> PitchMarker::markWithVowel(Iterator it_input_begin, Iterator it_input_end,
                                            Iterator it_vowel_begin, Iterator it_vowel_end,
                                            vector<double>* xcorr_std) const
{
  vector<Iterator> pitchmarks(1, it_input_begin);
  if (it_vowel_begin>=it_vowel_end || it_input_begin>=it_input_end) {
    return pitchmarks;
  }

  short win_size = it_vowel_end - it_vowel_begin;
  Iterator tmp_pitchmark = it_input_begin;
  vector<double> xcorr_win(win_size*2, 0.0), xcorr_max;
  pitchmarks.reserve((it_input_end-it_input_begin)/win_size);

  long dist = win_size/2, pre_dist = dist;
  while (tmp_pitchmark < it_input_end-(win_size/2*3)) {
    xcorr(tmp_pitchmark+(win_size/2), xcorr_win.begin(), it_vowel_begin, it_vowel_end);
    short margin_fore=win_size/4*3, margin_aft=win_size/4*7;
    vector<double>::iterator it_xcorr_max = max_element(xcorr_win.begin()+margin_fore, xcorr_win.begin()+margin_aft);
    dist = it_xcorr_max - xcorr_win.begin() - (win_size/2);
    if (xcorr_std != 0) {
      xcorr_max.push_back(*it_xcorr_max);
    }
    pre_dist = dist;
    pitchmarks.push_back(tmp_pitchmark+=dist);
  }

  if (xcorr_std != 0) {
    xcorr_std->assign(xcorr_max.size(), 0.0);
    double avg=0.0, stdev=0.0;
    for (size_t i=0; i<xcorr_max.size(); i++) {
      avg += xcorr_max[i] / xcorr_max.size();
    }
    for (size_t i=0; i<xcorr_max.size(); i++) {
      stdev += pow(xcorr_max[i]-avg, 2.0) / xcorr_max.size();
    }
    for (size_t i=0; i<xcorr_max.size(); i++) {
      xcorr_std->at(i) = (xcorr_max[i]-avg) / stdev;
    }
  }

  return pitchmarks;
}

template <class Iterator>
vector<Iterator> PitchMarker::markWithSelf(Iterator it_input_begin, Iterator it_input_end,
                                           Iterator it_base_begin, Iterator it_base_end) const
{
  vector<Iterator> pitchmarks(1, it_input_begin);
  if (it_base_begin>=it_base_end || it_input_begin>=it_input_end) {
    return pitchmarks;
  }

  short win_size = it_base_end - it_base_begin;
  Iterator tmp_pitchmark = it_input_begin;
  vector<double> xcorr_win(win_size*2, 0.0);
  pitchmarks.reserve((it_input_end-it_input_begin)/win_size);

  long dist = win_size/2;
  double xcorr_sum=0, xcorr_start=0;
  while (tmp_pitchmark < it_input_end-(win_size/2*3)) {
    xcorr(tmp_pitchmark+(win_size/2), xcorr_win.begin(), pitchmarks.back()-(win_size/2), pitchmarks.back()+(win_size/2));
    short margin_fore, margin_aft;
    if (dist>win_size/2) {
      margin_fore = win_size-params.pitch_margin;
      margin_aft = (dist*2)+params.pitch_margin;
    } else {
      margin_fore = (dist*2)-params.pitch_margin;
      margin_aft = win_size+params.pitch_margin;
    }
    if (margin_fore <= (win_size/4*3)) {
      margin_fore = (win_size/4*3)+1;
    }
    if (margin_aft >= (win_size/4*7)) {
      margin_aft = (win_size/4*7)-1;
    }
    dist = max_element(xcorr_win.begin()+margin_fore, xcorr_win.begin()+margin_aft) - xcorr_win.begin() - (win_size/2);
    pitchmarks.push_back(tmp_pitchmark+=dist);
  }

  return pitchmarks;
}

/*
 * accessor
 */
void PitchMarker::setInputWavParam(short ms_offs, short ms_ovrl, short ms_prec, short ms_blnk, unsigned long fs)
{
  this->it_input_wav_offs = this->input_wav.begin() + (fs/1000.0*ms_offs);
  this->it_input_wav_ovrl = this->it_input_wav_offs;
  if (ms_ovrl > 0) {
    this->it_input_wav_ovrl += fs / 1000.0 * ms_ovrl;
  }
  this->it_input_wav_prec = this->it_input_wav_offs;
  if (ms_prec > 0) {
    this->it_input_wav_prec += fs / 1000.0 * ms_prec;
  }

  long pos_blnk = fs / 1000.0 * ms_blnk;
  if (pos_blnk >= 0) {
    this->it_input_wav_blnk = this->input_wav.end() - pos_blnk;
  } else {
    this->it_input_wav_blnk = this->it_input_wav_offs - pos_blnk;
  }
}

long PitchMarker::getFadeStartSub() const
{
  return sub_fade_start;
}

vector<long> PitchMarker::getPitchMarks() const
{
  vector<long> tmp_pitchmarks(pitchmarks.begin(), pitchmarks.begin()+sub_fade_end+1);
  return tmp_pitchmarks;
}

/*
 * private
 */
template <class Iterator>
void PitchMarker::xcorr(Iterator it_input_begin, vector<double>::iterator it_output,
                        const Iterator it_base_begin, const Iterator it_base_end) const
{
  short win_size = it_base_end-it_base_begin, fftlen = 1;
  vector<double> filter = getWindow(win_size, 1);
  for (;fftlen<win_size*2; fftlen<<=1);

  fftw_complex *in1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));

  for (size_t i=0; i<fftlen; i++) {
    in1[i][0] = in1[i][1] = in2[i][0] = in2[i][1] = 0;
  }
  for (size_t i=0; i<win_size; i++) {
    in1[i][0] = *(it_base_begin+i) * filter[i];
    in2[i+win_size][0] = *(it_input_begin-(win_size/2)+i) * filter[i];
  }

  fftw_plan p1 = fftw_plan_dft_1d(fftlen, in1, out1, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p1);
  fftw_destroy_plan(p1);

  fftw_plan p2 = fftw_plan_dft_1d(fftlen, in2, out2, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p2);
  fftw_destroy_plan(p2);

  for (size_t i=0; i<fftlen; i++) {
    in3[i][0] = (out1[i][0]*out2[i][0])+(out1[i][1]*out2[i][1]);
    in3[i][1] = (out1[i][0]*out2[i][1])-(out1[i][1]*out2[i][0]);
  }

  fftw_plan p3 = fftw_plan_dft_1d(fftlen, in3, out3, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_execute(p3);
  fftw_destroy_plan(p3);

  for (size_t i=0; i<win_size*2; i++) {
    *(it_output+i) = out3[i][0];
  }

  fftw_free(in1);
  fftw_free(in2);
  fftw_free(in3);
  fftw_free(out1);
  fftw_free(out2);
  fftw_free(out3);
}
