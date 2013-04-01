#include "NoteArranger.h"

using namespace std;

void NoteArranger::arrange(Score *score)
{
  for (list<Note>::iterator it_notes=score->notes.begin(); it_notes!=score->notes.end(); ++it_notes) {
    vector<short> velocities;
    if (it_notes->getVelocityPointNum()>0 || (velocities=it_notes->getVelocities()).size()==0)
      continue;

    unsigned short range = (velocities.size()-1) / 2;
    unsigned short ms_front_edge=nak::ms_front_edge, ms_back_edge=nak::ms_back_edge;
    if (it_notes!=score->notes.begin() && boost::prior(it_notes)->getEnd()==it_notes->getStart() && it_notes->getOvrl()>0)
      ms_front_edge = it_notes->getOvrl();
    if (it_notes!=--score->notes.end() && boost::next(it_notes)->getStart()==it_notes->getEnd() && boost::next(it_notes)->getOvrl()>0)
      ms_back_edge = boost::next(it_notes)->getOvrl();

    it_notes->addVelocityPoint(0, 0);
    it_notes->addVelocityPoint(ms_front_edge, it_notes->getBaseVelocity());
    it_notes->addVelocityPoint(-ms_back_edge, it_notes->getBaseVelocity());
    it_notes->addVelocityPoint(-1, 0);
  }
}
