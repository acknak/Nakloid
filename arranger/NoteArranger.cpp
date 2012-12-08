#include "NoteArranger.h"

using namespace std;

const unsigned short NoteArranger::ms_front_edge = 30; //ms
const unsigned short NoteArranger::ms_back_edge = 30; //ms

void NoteArranger::arrange(Score *score)
{
  list<Note> notes = score->getNotesList();
  for (list<Note>::iterator it_notes=notes.begin(); it_notes!=notes.end(); ++it_notes) {
    vector<unsigned char> velocities = it_notes->getVelocities();
    unsigned short range = (velocities.size()-1) / 2;
    edge_front(velocities.begin(), min(ms_front_edge, range));
    edge_back(velocities.rbegin(), min(ms_back_edge, range));
    it_notes->setVelocities(velocities);
  }
  score->setNotes(notes);
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
