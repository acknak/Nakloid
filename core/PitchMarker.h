#ifndef PitchMarker_h
#define PitchMarker_h

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>
#include "core-inl.h"
#include "../format/Wav.h"

// Refference Object
class PitchMarker {
 public:
  static struct Parameters {
    Parameters() {
      pitch_margin = 10;
      xcorr_threshold = 0.95;
    };
    short pitch_margin;
    double xcorr_threshold;
  } params;

  explicit PitchMarker(const std::vector<double>& input_wav)
   :input_wav(input_wav), it_input_wav_offs(input_wav.begin()), it_input_wav_blnk(input_wav.end()), sub_fade_start(0), sub_fade_end(0){}
  PitchMarker(const std::vector<double>& input_wav, short ms_offs, short ms_ovrl, short ms_cons, short ms_blnk, unsigned long fs);
  ~PitchMarker(){}

  bool mark(const std::vector<double>& vowel_wav);
  bool mark(const std::vector<double>& fore_vowel_wav, const std::vector<double>& aft_vowel_wav);
  bool mark(double hz, unsigned long fs);

  // accessor
  void setInputWavParam(short ms_offs, short ms_ovrl, short ms_cons, short ms_blnk, unsigned long fs);
  std::vector<long> getPitchMarks() const;
  long getFadeStartSub() const;

 private:
  PitchMarker(const PitchMarker& other);
  PitchMarker& operator=(const PitchMarker& other);

  std::vector<double> input_wav;
  long sub_fade_start, sub_fade_end;
  std::vector<double>::const_iterator it_input_wav_offs;
  std::vector<double>::const_iterator it_input_wav_ovrl;
  std::vector<double>::const_iterator it_input_wav_cons;
  std::vector<double>::const_iterator it_input_wav_blnk;
  std::vector<long> pitchmarks;

  template <class Iterator>
  std::vector<Iterator> markWithVowel(const Iterator it_input_begin, const Iterator it_input_end,
                                      const Iterator it_vowel_begin, const Iterator it_vowel_end) const;
  template <class Iterator>
  std::vector<Iterator> markWithSelf(const Iterator it_input_begin, const Iterator it_input_end,
                                     const Iterator it_base_begin, const Iterator it_base_end) const;
};

#endif
