#ifndef core_inl_h
#define core_inl_h

#include "../format/Wav.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

inline long ms2pos(long ms, WavHeader header) { return (long)(ms / 1000.0 * header.dwSamplesPerSec); }
inline long pos2ms(long pos, WavHeader header) { return (long)(pos / (double)header.dwSamplesPerSec * 1000); }
inline double dB2val(const std::pair<bool, double>& dB) { return (dB.second>0) ? 0.0 : pow(10, dB.second / 20)*(dB.first ? 1 : -1); };
inline double sinc(double x){ return sin(M_PI*x) / (M_PI*x); }

inline std::pair<bool, double> val2dB(double wav_value)
{
  if (wav_value >= 1.0) {
    wav_value = 32768.0 / 32767.0;
  }
  else if (wav_value <= -1.0) {
    wav_value = -32769.0 / 32768.0;
  }
  else if (wav_value == 0){
    return std::make_pair(true, 1.0);
  }
  return std::make_pair(wav_value>0, log10(abs(wav_value)) * 20);
}

inline std::vector<double> getWindow(long len, unsigned char lobe)
{
  std::vector<double> filter(len, 0);
  if (lobe > 1) {
    // Lanczos Window
    long pos_half = filter.size() / 2;
    if (len % 2 > 0) {
      filter[pos_half] = 1.0;
      ++pos_half;
    }
    for (size_t i = 0; i<filter.size() - pos_half; i++) {
      double x = (i + 1.0) * lobe / pos_half;
      filter[pos_half + i] = sinc(x) * sinc(x / lobe);
    }
    reverse_copy(filter.begin() + pos_half, filter.end(), filter.begin());
  }
  else {
    // Hann Window
    for (size_t i = 0; i<filter.size(); ++i) {
      double x = (i + 1.0) / (filter.size() + 1.0);
      filter[i] = 0.5 - (0.5 * cos(2 * M_PI*x));
    }
  }
  return filter;
}

inline std::vector<double> getWindow(long len, unsigned char lobe, long output_pitch)
{
  if (lobe > 1) {
    getWindow(len, lobe);
  }
  /*
   * F(x,n) = sin(%pi*x)^n;
   * n = -log(2)/log(sin(%pi*x)); (F(x,n)=0.5)
   */
  double coeff = -log(2) / log(sin(M_PI*(0.5-(output_pitch/2.0/len))));
  std::vector<double> filter(len, 0.0);
  for (size_t i=0; i<len/2+1; i++) {
    filter[i] = filter[len-i-1] = pow(sin(M_PI*i/len), coeff);
  }
  if (len%2==1) {
    filter[len/2+1] = 1.0;
  }
  return filter;
}

inline double getRMS(const std::vector<double>::const_iterator from, const std::vector<double>::const_iterator to)
{
  double rms = 0.0;
  for (std::vector<double>::const_iterator it = from; it != to; ++it) {
    rms += pow((double)*it, 2) / (to - from);
  }
  return sqrt(rms);
}

inline double getRMS(const std::vector<double>& wav){ return getRMS(wav.begin(), wav.end()); }

#endif
