#ifndef PitchMarker_h
#define PitchMarker_h

#include <list>
#include <cmath>
#include <vector>
#include <fstream>
#include <numeric>
#include <iomanip>
#include <iostream>
#include <algorithm>
 
#include "fftw3.h"
#include "../Utilities.h"
#include "../parser/WavData.h"

// Refference Object
class PitchMarker {
 public:
  PitchMarker();
  explicit PitchMarker(short win_size);
  PitchMarker(short pitch, unsigned long fs);
  virtual ~PitchMarker();

  bool mark(WavData input);
  bool mark(std::list<short> input);
  bool mark(std::vector<short> input);
  void debug(std::string output);

  // accessor
  short getWinSize();
  void setWinSize(short win_size);
  void setWinSize(double pitch, unsigned long fs);
  long getPosOffs();
  long getPosCons();
  long getPosBlnk();
  void setRange(unsigned short offs, unsigned short cons, unsigned short blnk, unsigned long fs);
  std::list<long> getMarkList();
  std::vector<long> getMarkVector();
  std::vector<short> getTargetWav();
  void setTargetWav(std::vector<short> target_wav);

 private:
  PitchMarker(const PitchMarker& other);
  PitchMarker& operator=(const PitchMarker& other);

  template<typename it>
  std::vector<double> xcorr(it it_start, it it_base, short exp_dist);
  template<typename it>
  std::vector<double> xcorr(it it_start, std::vector<short> target_wav, short exp_dist);
  short win_size;
  long pos_offs;
  long pos_cons;
  long pos_blnk;
  std::list<long> mark_list;
  std::vector<short> target_wav;
};

#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
