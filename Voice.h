#ifndef Voice_h
#define Voice_h

#include <string>
#include "PSOLA/BaseWav.h"

typedef struct {
  std::string path;
  std::string filename;
  std::string pron;
  std::string prefix;
  std::string suffix;
  bool is_vcv;
  short offs; // offset(left blank)
  short cons; // consonant part(unaltered range)
  short blnk; // blank(right blank)
  short prec; // preceding utterance
  short ovrl; // overlap range
  double frq;
  BaseWavsContainer bwc;
} Voice;

#endif
