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
  PitchMarker();
  virtual ~PitchMarker();

  bool mark(const std::vector<double>& vowel_wav);
  bool mark(const std::vector<double>& fore_vowel_wav, const std::vector<double>& aft_vowel_wav);
  bool mark(double hz, unsigned long fs);

  template <class Iterator>
  std::vector<Iterator> mark(Iterator it_vowel_begin, Iterator it_vowel_end,
                             Iterator it_target_begin, Iterator it_target_end,
                             Iterator it_wav_end, bool autocorrelation) const;

  // accessor
  void setInputWav(const std::vector<double>& input_wav);
  void setInputWav(const std::vector<double>& input_wav, short ms_offs, short ms_ovrl, short ms_prec, short ms_blnk, unsigned long fs);
  const std::vector<long>& getPitchMarks() const;

 protected:
  template <class Iterator>
  void xcorr(Iterator it_vowel_begin, Iterator it_vowel_end,
             Iterator it_target_begin, std::vector<double>::iterator it_output)const;

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
