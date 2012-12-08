#include "Arranger.h"

unsigned long Arranger::ms2pos(unsigned long ms, WavFormat format)
{
  return (unsigned long)(ms/1000.0*format.dwSamplesPerSec);
}

unsigned long Arranger::pos2ms(unsigned long pos, WavFormat format)
{
  return (unsigned long)(pos/(double)format.dwSamplesPerSec*1000);
}
