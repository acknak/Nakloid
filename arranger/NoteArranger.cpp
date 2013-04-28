#include "NoteArranger.h"

using namespace std;

void NoteArranger::arrange(Score *score)
{
  for (list<Note>::iterator it_notes=score->notes.begin(); it_notes!=score->notes.end(); ++it_notes) {
    vector<short> velocities;
    if (it_notes->getVelocityPointNum() > 0)
      continue;

    unsigned short ms_front_edge=nak::ms_front_edge, ms_back_edge=nak::ms_back_edge;

    it_notes->addVelocityPoint(0, 0);
    it_notes->addVelocityPoint(ms_front_edge, it_notes->getBaseVelocity());
    it_notes->addVelocityPoint(-ms_back_edge, it_notes->getBaseVelocity());
    it_notes->addVelocityPoint(-1, 0);
  }
}
