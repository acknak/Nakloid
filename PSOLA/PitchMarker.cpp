#include "PitchMarker.h"

#pragma comment(lib, "libfftw3-3.lib")

using namespace std;

PitchMarker::PitchMarker(const vector<double>& input_wav):pos_offs(0)
{
  this->input_wav = input_wav;
  this->pos_offs = 0;
  this->it_input_wav_offs = this->input_wav.begin();
  this->it_input_wav_blnk = this->input_wav.end();
}

PitchMarker::PitchMarker(const vector<double>& input_wav, short ms_offs, short ms_ovrl, short ms_prec, short ms_blnk, unsigned long fs)
{
  this->input_wav = input_wav;
  setInputWavParam(ms_offs, ms_ovrl, ms_prec, ms_blnk, fs);
}

PitchMarker::~PitchMarker(){}

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

  long pos_fore_mark_end, pitch_fore_mark_end, pos_aft_mark_start, pitch_aft_mark_start;
  vector<double>::const_iterator it_cons_start, it_cons_end, it_base_start, it_base_end;
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
    it_base_start = *(tmp_pitchmarks.end()-3);
    it_base_end = *(tmp_pitchmarks.end()-1);
    it_cons_start = it_base_end;
  }
  {
    vector<double>::const_reverse_iterator rit_mark_start, rit_input_wav_offs(it_input_wav_blnk), rit_input_wav_prec(it_input_wav_prec);
    {
      // find start point
      vector<double> xcorr_win(win_size*2, 0.0);
      rit_mark_start = max_element(rit_input_wav_offs+(win_size/2), rit_input_wav_offs+(win_size/2*3));
      xcorr(rit_mark_start, xcorr_win.begin(), aft_vowel_wav.rbegin(), aft_vowel_wav.rend());
      rit_mark_start += max_element(xcorr_win.begin(), xcorr_win.end()) - xcorr_win.begin() - (win_size/2);
    }
    // aft vowel pitch mark
    vector<vector<double>::const_reverse_iterator> tmp_pitchmarks =
      markWithVowel(rit_mark_start, rit_input_wav_prec, aft_vowel_wav.rbegin(), aft_vowel_wav.rend());
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[i]);
    }
    it_cons_end = (*(tmp_pitchmarks.end()-3)).base();
    pos_aft_mark_start = pitchmarks.back();
    pitch_aft_mark_start = *(----pitchmarks.end()) - *(--pitchmarks.end());
  }
  {
    // consonant pitch mark
    vector<vector<double>::const_iterator> tmp_pitchmarks =
      markWithSelf(it_cons_start, it_cons_end, it_base_start, it_base_end, true);
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(tmp_pitchmarks[i] - input_wav.begin());
    }
    pos_fore_mark_end = pitchmarks.back();
    pitch_fore_mark_end = *(--pitchmarks.end()) - *(----pitchmarks.end());
    long pos_len=pos_aft_mark_start-pos_fore_mark_end, pitch_avg=(pitch_fore_mark_end+pitch_aft_mark_start)/2;
    if (pos_len > pitch_avg) {
      long pos_div = (pos_len%pitch_avg) / (pos_len/pitch_avg);
      long pos_rem = (pos_len%pitch_avg) % (pos_len/pitch_avg);
      long tmp_pos = 0;
      while ((tmp_pos+=pitch_avg+pos_div) < pos_len) {
        pitchmarks.push_back(pos_fore_mark_end+(--pos_rem>=0?++tmp_pos:tmp_pos));
      }
    }

  }

  sort(pitchmarks.begin(), pitchmarks.end());
  pitchmarks.erase(unique(pitchmarks.begin(), pitchmarks.end()), pitchmarks.end());

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

  vector<double>::const_reverse_iterator rit_input_wav_offs(it_input_wav_blnk), rit_input_wav_blnk(it_input_wav_offs), rit_base_start, rit_base_end;
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
    vector<vector<double>::const_reverse_iterator> tmp_pitchmarks =
      markWithVowel(rit_mark_start, rit_input_wav_blnk, vowel_wav.rbegin(), vowel_wav.rend());
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[i]);
    }
    rit_base_start = *(tmp_pitchmarks.end()-3);
    rit_base_end = *(tmp_pitchmarks.end()-1);
  }
  {
    // consonant pitch mark
    vector<double>::const_reverse_iterator rit_input_wav_blnk(it_input_wav_offs);
    vector<vector<double>::const_reverse_iterator> tmp_pitchmarks =
      markWithSelf(rit_base_end, rit_input_wav_blnk, rit_base_start, rit_base_end, false);
    for (size_t i=0; i<tmp_pitchmarks.size(); i++) {
      pitchmarks.push_back(input_wav.rend()-tmp_pitchmarks[i]);
    }
  }

  sort(pitchmarks.begin(), pitchmarks.end());
  pitchmarks.erase(unique(pitchmarks.begin(), pitchmarks.end()), pitchmarks.end());

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
  vector<Iterator> pitchmarks(1, it_input_begin);
  if (it_vowel_begin>=it_vowel_end || it_input_begin>=it_input_end) {
    return pitchmarks;
  }

  short win_size = it_vowel_end - it_vowel_begin;
  Iterator tmp_pitchmark = it_input_begin;
  vector<double> xcorr_win(win_size*2, 0.0);
  pitchmarks.reserve((it_input_end-it_input_begin)/win_size);

  long dist = win_size/2, pre_dist = dist;
  while (tmp_pitchmark < it_input_end-(win_size/2*3)) {
    xcorr(tmp_pitchmark+(win_size/2), xcorr_win.begin(), it_vowel_begin, it_vowel_end);
    short margin_fore=win_size/4*3, margin_aft=win_size/4*7;
    dist = max_element(xcorr_win.begin()+margin_fore, xcorr_win.begin()+margin_aft) - xcorr_win.begin() - (win_size/2);
    if ((pre_dist<dist-nak::pitch_margin || pre_dist>dist+nak::pitch_margin) && pitchmarks.size()>2) {
      return pitchmarks;
    }
    pre_dist = dist;
    pitchmarks.push_back(tmp_pitchmark+=dist);
  }

  return pitchmarks;
}

template <class Iterator>
vector<Iterator> PitchMarker::markWithSelf(Iterator it_input_begin, Iterator it_input_end,
                                           Iterator it_base_begin, Iterator it_base_end, bool breaker) const
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
  double xcorr_sum = 0;
  while (tmp_pitchmark < it_input_end-(win_size/2*3)) {
    xcorr(tmp_pitchmark+(win_size/2), xcorr_win.begin(), pitchmarks.back()-(win_size/2), pitchmarks.back()+(win_size/2));
    short margin_fore, margin_aft;
    if (dist>win_size/2) {
      margin_fore = win_size-nak::pitch_margin;
      margin_aft = (dist*2)+nak::pitch_margin;
    } else {
      margin_fore = (dist*2)-nak::pitch_margin;
      margin_aft = win_size+nak::pitch_margin;
    }
    if (margin_fore <= (win_size/4*3)) {
      margin_fore = (win_size/4*3)+1;
    }
    if (margin_aft >= (win_size/4*7)) {
      margin_aft = (win_size/4*7)-1;
    }
    dist = max_element(xcorr_win.begin()+margin_fore, xcorr_win.begin()+margin_aft) - xcorr_win.begin() - (win_size/2);
    if (breaker) {
      double tmp_max = *max_element(xcorr_win.begin()+margin_fore, xcorr_win.begin()+margin_aft);
      if (xcorr_sum == 0) {
        xcorr_sum = tmp_max;
      } else if (xcorr_sum*nak::xcorr_threshold > tmp_max) {
        break;
      }
    }
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
  if (pos_blnk > 0) {
    this->it_input_wav_blnk = this->input_wav.end() - pos_blnk;
  } else {
    this->it_input_wav_blnk = this->it_input_wav_offs - pos_blnk;
  }
}

const vector<long>& PitchMarker::getPitchMarks() const
{
  return pitchmarks;
}

/*
 * protected
 */
template <class Iterator>
void PitchMarker::xcorr(Iterator it_input_begin, vector<double>::iterator it_output,
                        const Iterator it_base_begin, const Iterator it_base_end) const
{
  short win_size = it_base_end - it_base_begin;
  int fftlen = win_size * 2;
  vector<double> filter = nak::getHann(win_size);

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

  for (size_t i=0; i<fftlen; i++) {
    *(it_output+i) = out3[i][0];
  }

  fftw_free(in1);
  fftw_free(in2);
  fftw_free(in3);
  fftw_free(out1);
  fftw_free(out2);
  fftw_free(out3);
}
