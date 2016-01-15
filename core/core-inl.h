#ifndef core_inl_h
#define core_inl_h

#include "../format/Wav.h"
#include "fftw3compat.h"

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
  if (output_pitch > len) {
    output_pitch = len;
  }
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

template <class Iterator>
inline void xcorr(const Iterator it_input_begin, std::vector<double>::iterator it_output,
                  const Iterator it_base_begin, const Iterator it_base_end)
{
  short win_size = it_base_end - it_base_begin, fftlen = 1;
  vector<double> filter = getWindow(win_size, 1);
  for (; fftlen<win_size * 2; fftlen <<= 1);

  fftw_complex *in1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *in3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out1 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out2 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));
  fftw_complex *out3 = (fftw_complex*)(fftw_malloc(sizeof(fftw_complex) * fftlen));

  for (size_t i = 0; i<fftlen; i++) {
    in1[i][0] = in1[i][1] = in2[i][0] = in2[i][1] = 0;
  }
  for (size_t i = 0; i<win_size; i++) {
    in1[i][0] = *(it_base_begin + i) * filter[i];
    in2[i + win_size][0] = *(it_input_begin - (win_size / 2) + i) * filter[i];
  }

  fftw_plan p1 = fftw_plan_dft_1d(fftlen, in1, out1, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p1);
  fftw_destroy_plan(p1);

  fftw_plan p2 = fftw_plan_dft_1d(fftlen, in2, out2, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p2);
  fftw_destroy_plan(p2);

  for (size_t i = 0; i<fftlen; i++) {
    in3[i][0] = (out1[i][0] * out2[i][0]) + (out1[i][1] * out2[i][1]);
    in3[i][1] = (out1[i][0] * out2[i][1]) - (out1[i][1] * out2[i][0]);
  }

  fftw_plan p3 = fftw_plan_dft_1d(fftlen, in3, out3, FFTW_BACKWARD, FFTW_ESTIMATE);
  fftw_execute(p3);
  fftw_destroy_plan(p3);

  for (size_t i = 0; i<win_size * 2; i++) {
    *(it_output + i) = out3[i][0];
  }

  fftw_free(in1);
  fftw_free(in2);
  fftw_free(in3);
  fftw_free(out1);
  fftw_free(out2);
  fftw_free(out3);
}
#endif
