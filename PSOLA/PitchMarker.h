#ifndef PitchMarker_h
#define PitchMarker_h

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>
#include "fftw3.h"
#include "../Utilities.h"
#include "../parser/WavData.h"

// Refference Object
class PitchMarker {
 public:
  PitchMarker(const std::vector<double>& input_wav);
  PitchMarker(const std::vector<double>& input_wav, short ms_offs, short ms_ovrl, short ms_prec, short ms_blnk, unsigned long fs);
  virtual ~PitchMarker();

  bool mark(const std::vector<double>& vowel_wav);
  bool mark(const std::vector<double>& fore_vowel_wav, const std::vector<double>& aft_vowel_wav);
  bool mark(double hz, unsigned long fs);

  template <class Iterator>
  std::vector<Iterator> markWithVowel(Iterator it_input_begin, Iterator it_input_end,
                                      Iterator it_vowel_begin, Iterator it_vowel_end) const;
  template <class Iterator>
  std::vector<Iterator> markWithSelf(Iterator it_input_begin, Iterator it_input_end,
                                     Iterator it_base_begin, Iterator it_base_end, bool breaker) const;

  // accessor
  void setInputWavParam(short ms_offs, short ms_ovrl, short ms_prec, short ms_blnk, unsigned long fs);
  const std::vector<long>& getPitchMarks() const;

 protected:
  template <class Iterator>
  void xcorr(Iterator it_input_begin, std::vector<double>::iterator it_output,
             Iterator it_base_begin, Iterator it_base_end) const;
  std::vector<double> input_wav;
  long pos_offs;
  std::vector<double>::const_iterator it_input_wav_offs;
  std::vector<double>::const_iterator it_input_wav_ovrl;
  std::vector<double>::const_iterator it_input_wav_prec;
  std::vector<double>::const_iterator it_input_wav_blnk;
  std::vector<long> pitchmarks;

 private:
  PitchMarker(const PitchMarker& other);
  PitchMarker& operator=(const PitchMarker& other);
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
