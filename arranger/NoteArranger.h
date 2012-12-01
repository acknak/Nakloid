#ifndef NoteArranger_h
#define NoteArranger_h

#include <list>
#include <vector>
#include <algorithm>
#include "Arranger.h"
#include "../Note.h"
#include "../Score.h"
#include "../PSOLA/BaseWav.h"

class NoteArranger : Arranger {
 public:
  const static unsigned short ms_front_edge;
  const static unsigned short ms_back_edge;

  static void arrange(Score *score);

 private:
  static void edge_front(std::vector<unsigned char>::iterator it_start, unsigned short range);
  static void edge_back(std::vector<unsigned char>::reverse_iterator it_rstart, unsigned short range);
};

#endif
