#include "PitchArranger.h"

using namespace std;

void PitchArranger::arrange(Score *score)
{
  vector<float> pitches = score->getPitches();

  for (list<Note>::iterator it_notes=score->notes.begin();it_notes!=score->notes.end();++it_notes) {
    if (nak::vibrato)
      vibrato(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd());
    if (nak::interpolation)
      interpolation(pitches.begin(), it_notes->getPronStart(), it_notes->getPronEnd(), it_notes->getBasePitchHz());
    if (nak::overshoot)
      if (it_notes!=score->notes.begin() && it_notes->getStart()==boost::prior(it_notes)->getEnd())
        overshoot(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd(), *(pitches.begin()+boost::prior(it_notes)->getEnd()-1));
    if (nak::preparation)
      if (it_notes!=--score->notes.end() && it_notes->getEnd()==boost::next(it_notes)->getStart())
        preparation(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd(), *(pitches.begin()+boost::next(it_notes)->getStart()));
  }

  score->setPitches(pitches);
}

void PitchArranger::vibrato(vector<float>::iterator it_pitches_begin, vector<float>::iterator it_pitches_end)
{
  if ((it_pitches_end-it_pitches_begin) > nak::ms_vibrato_offset) {
    unsigned long vibrato_length = (it_pitches_end-it_pitches_begin) - nak::ms_vibrato_offset;
    for (unsigned long i=0; i<vibrato_length; i++)
      *(it_pitches_begin+nak::ms_vibrato_offset+i) += sin(2*M_PI*i/nak::ms_vibrato_width) * nak::pitch_vibrato;
  }
}

void PitchArranger::overshoot(vector<float>::iterator it_pitches_begin, vector<float>::iterator it_pitches_end, float target_pitch)
{
  float diff = (*it_pitches_begin-target_pitch);

  if (it_pitches_end-it_pitches_begin > nak::ms_overshoot)
    for (int i=0; i<nak::ms_overshoot/2; i++) {
      *(it_pitches_begin+i) +=
        -diff + ((diff+(nak::pitch_overshoot*((diff>0)?1:-1))) / (nak::ms_overshoot/2) * i);
      *(it_pitches_begin+(nak::ms_overshoot/2)+i) +=
        (nak::pitch_overshoot*((diff>0)?1:-1)) + ((nak::pitch_overshoot*((diff>0)?-1:1))/(nak::ms_overshoot/2) * i);
    }
  else
    for (int i=0; i<it_pitches_end-it_pitches_begin; i++)
      *(it_pitches_begin+i) += -diff + (diff/(it_pitches_end-it_pitches_begin)*i);
}

void PitchArranger::preparation(vector<float>::iterator it_pitches_begin, vector<float>::iterator it_pitches_end, float target_pitch)
{
  vector<float>::reverse_iterator rit_pitches_begin(it_pitches_end);
  vector<float>::reverse_iterator rit_pitches_end(it_pitches_begin);
  float diff = (*rit_pitches_begin-target_pitch) / 2;

  if (rit_pitches_end-rit_pitches_begin > nak::ms_preparation)
    for (int i=0; i<nak::ms_preparation/2; i++) {
      *(rit_pitches_begin+i) +=
        -diff + ((diff+(nak::pitch_preparation*((diff>0)?1:-1))) / (nak::ms_preparation/2) * i);
      *(rit_pitches_begin+(nak::ms_preparation/2)+i) +=
        (nak::pitch_preparation*((diff>0)?1:-1)) + ((nak::pitch_preparation*((diff>0)?-1:1))/(nak::ms_preparation/2) * i);
    }
  else
    for (int i=0; i<rit_pitches_end-rit_pitches_begin; i++)
      *(rit_pitches_begin+i) += -diff + (diff/(rit_pitches_end-rit_pitches_begin)*i);
}

void PitchArranger::interpolation(vector<float>::iterator it_pitches, unsigned long ms_pron_start, unsigned long ms_pron_end, float target_pitch)
{
  for (unsigned long i=0; i<ms_pron_end-ms_pron_start; i++)
    if (*(it_pitches+ms_pron_start+i) == 0)
      *(it_pitches+ms_pron_start+i) = target_pitch;
}
