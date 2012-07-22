#include "PitchArranger.h"

#define M_PI 3.1415926535897932384626433832795

using namespace std;

const short PitchArranger::overshoot_length = 100; //ms
const double PitchArranger::overshoot_height = 10; //hz
const short PitchArranger::preparation_length = 100; //ms
const double PitchArranger::preparation_height = 10; //hz
const short PitchArranger::vibrato_offset = 500; //ms
const short PitchArranger::vibrato_width = 200; //ms
const double PitchArranger::vibrato_depth = 5; //hz

void PitchArranger::vibrato(vector<double> *guide_pitches)
{
  if (guide_pitches->size() > vibrato_offset)
    for (int i=0; i<guide_pitches->size()-vibrato_offset; i++)
      (*guide_pitches)[i+vibrato_offset] += sin(2*M_PI*i/vibrato_width) * vibrato_depth;
}

void PitchArranger::overshoot(vector<double> *guide_pitches, double pitch_from, double pitch_to)
{
  double diff = (pitch_to-pitch_from) / 2;
  if (guide_pitches->size()/2.0 > overshoot_length)
    for (int i=0; i<overshoot_length/2; i++) {
      (*guide_pitches)[i] +=
        -diff + ((diff+(overshoot_height*((diff>0)?1:-1))) / (overshoot_length/2) * i);
      (*guide_pitches)[i+(overshoot_length/2)] +=
        (overshoot_height*((diff>0)?1:-1)) + ((overshoot_height*((diff>0)?-1:1))/(overshoot_length/2) * i);
    }
  else
    for (int i=0; i<guide_pitches->size()/2.0; i++)
      (*guide_pitches)[i] += -diff + (diff/(guide_pitches->size()/2)*i);
}

void PitchArranger::preparation(vector<double> *guide_pitches, double pitch_from, double pitch_to)
{
  if (guide_pitches->size() < vibrato_offset)
    return;

  //double diff = (pitch_from-pitch_to) / 2;
  double diff = pitch_from - pitch_to;
  if (guide_pitches->size()/2.0 > preparation_length){
    for (int i=0; i<preparation_length/2; i++) {
      (*guide_pitches)[guide_pitches->size()-i-1] +=
        -diff + ((diff+(preparation_height*((diff>0)?1:-1)))/(preparation_length/2)*i);
      (*guide_pitches)[guide_pitches->size()-(i+(preparation_length/2))-1] +=
        (preparation_height*((diff>0)?1:-1)) + ((preparation_height*((diff>0)?-1:1))/(preparation_length/2)*i);
    }
  } else
    for (int i=0; i<guide_pitches->size(); i++)
      (*guide_pitches)[guide_pitches->size()-i-1] += diff + (-diff/(guide_pitches->size()/2)*i);
}
