#ifndef NoteArranger_h
#define NoteArranger_h

#include <list>
#include <vector>
#include <algorithm>
#include "../score/Note.h"
#include "../score/Score.h"
#include "../Utilities.h"
#include "../PSOLA/BaseWav.h"

class NoteArranger {
 public:
  static void arrange(Score *score);
};

#endif
