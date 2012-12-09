#include "NoteArranger.h"

using namespace std;

const unsigned short NoteArranger::ms_front_edge = 30; //ms
const unsigned short NoteArranger::ms_back_edge = 30; //ms

void NoteArranger::arrange(Score *score)
{
  for (list<Note>::iterator it_notes=score->notes.begin(); it_notes!=score->notes.end(); ++it_notes) {
    vector<unsigned char> velocities = it_notes->getVelocities();
    unsigned short range = (velocities.size()-1) / 2;
    unsigned short ms_front_edge=NoteArranger::ms_front_edge, ms_back_edge=NoteArranger::ms_back_edge;
    if (it_notes!=score->notes.begin() && boost::prior(it_notes)->getEnd()==it_notes->getStart() && it_notes->getOvrl()>0)
      ms_front_edge = it_notes->getOvrl();
    if (it_notes!=--score->notes.end() && boost::next(it_notes)->getStart()==it_notes->getEnd() && boost::next(it_notes)->getOvrl()>0)
      ms_back_edge = boost::next(it_notes)->getOvrl();
    //edge_front(velocities.begin(), min(ms_front_edge, range));
    edge_back(velocities.rbegin(), min(ms_back_edge, range));
    it_notes->setVelocities(velocities);
  }
}

void NoteArranger::edge_front(std::vector<unsigned char>::iterator it_start, unsigned short range)
{
  for (int i=1; i<=range; i++,++it_start)
    *it_start *= i/(double)range;
}

void NoteArranger::edge_back(std::vector<unsigned char>::reverse_iterator it_rstart, unsigned short range)
{
  for (int i=1; i<=range; i++,++it_rstart)
    *it_rstart *= i/(double)range;
}
