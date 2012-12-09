#include "PitchArranger.h"

using namespace std;

const unsigned short PitchArranger::overshoot_length = 100; //ms
const double PitchArranger::overshoot_height = 5; //hz
const unsigned short PitchArranger::preparation_length = 100; //ms
const double PitchArranger::preparation_height = 5; //hz
const unsigned short PitchArranger::vibrato_offset = 400; //ms
const unsigned short PitchArranger::vibrato_width = 200; //ms
const double PitchArranger::vibrato_depth = 3; //hz

void PitchArranger::arrange(Score *score)
{
  vector<double> pitches = score->getPitches();

  for (list<Note>::iterator it_notes=score->notes.begin();it_notes!=score->notes.end();++it_notes) {
    vibrato(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd());
    if (it_notes!=score->notes.begin() && it_notes->getStart()==boost::prior(it_notes)->getEnd())
      overshoot(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd(), *(pitches.begin()+boost::prior(it_notes)->getEnd()-1));
    if (it_notes!=--score->notes.end() && it_notes->getEnd()==boost::next(it_notes)->getStart())
      preparation(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd(), *(pitches.begin()+boost::next(it_notes)->getStart()));
  }

  score->setPitches(pitches);
}

void PitchArranger::vibrato(vector<double>::iterator it_pitches_begin, vector<double>::iterator it_pitches_end)
{
  if ((it_pitches_end-it_pitches_begin) > vibrato_offset) {
    unsigned long vibrato_length = (it_pitches_end-it_pitches_begin) - vibrato_offset;
    for (int i=0; i<vibrato_length; i++)
      *(it_pitches_begin+vibrato_offset+i) += sin(2*M_PI*i/vibrato_width) * vibrato_depth;
  }
}

void PitchArranger::overshoot(vector<double>::iterator it_pitches_begin, vector<double>::iterator it_pitches_end, double target_pitch)
{
  double diff = (*it_pitches_begin-target_pitch);

  if (it_pitches_end-it_pitches_begin > overshoot_length)
    for (int i=0; i<overshoot_length/2; i++) {
      *(it_pitches_begin+i) +=
        -diff + ((diff+(overshoot_height*((diff>0)?1:-1))) / (overshoot_length/2) * i);
      *(it_pitches_begin+(overshoot_length/2)+i) +=
        (overshoot_height*((diff>0)?1:-1)) + ((overshoot_height*((diff>0)?-1:1))/(overshoot_length/2) * i);
    }
  else
    for (int i=0; i<it_pitches_end-it_pitches_begin; i++)
      *(it_pitches_begin+i) += -diff + (diff/(it_pitches_end-it_pitches_begin)*i);
}

void PitchArranger::preparation(vector<double>::iterator it_pitches_begin, vector<double>::iterator it_pitches_end, double target_pitch)
{
  vector<double>::reverse_iterator rit_pitches_begin(it_pitches_end);
  vector<double>::reverse_iterator rit_pitches_end(it_pitches_begin);
  double diff = (*rit_pitches_begin-target_pitch) / 2;

  if (rit_pitches_end-rit_pitches_begin > preparation_length)
    for (int i=0; i<preparation_length/2; i++) {
      *(rit_pitches_begin+i) +=
        -diff + ((diff+(preparation_height*((diff>0)?1:-1))) / (preparation_length/2) * i);
      *(rit_pitches_begin+(preparation_length/2)+i) +=
        (preparation_height*((diff>0)?1:-1)) + ((preparation_height*((diff>0)?-1:1))/(preparation_length/2) * i);
    }
  else
    for (int i=0; i<rit_pitches_end-rit_pitches_begin; i++)
      *(rit_pitches_begin+i) += -diff + (diff/(rit_pitches_end-rit_pitches_begin)*i);
}
