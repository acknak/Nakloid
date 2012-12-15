#ifndef NoteArranger_h
#define NoteArranger_h

#include <list>
#include <vector>
#include <algorithm>
#include <boost/utility.hpp>
#include "../Note.h"
#include "../Score.h"
#include "../Utilities.h"
#include "../PSOLA/BaseWav.h"

class NoteArranger {
 public:
  static void arrange(Score *score);

 private:
  static void sharpen_front(std::vector<unsigned char>::iterator it_start, unsigned short range);
  static void sharpen_back(std::vector<unsigned char>::reverse_iterator it_rstart, unsigned short range);
};

#endif
