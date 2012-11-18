#include "Note.h"

using namespace std;

Note::Note():start(0),end(0),pitch(0),pron(""),velocity(100),is_prec(false),prec(0),is_ovrl(false),ovrl(0){}

Note::Note(unsigned long start, unsigned char pitch, short velocity)
  :start(0),end(0),pitch(0),pron(""),velocity(100),is_prec(false),prec(0),is_ovrl(false),ovrl(0)
{
  this->setStart(start);
  this->setPitch(pitch);
  this->setVelocity(velocity);
}

Note::Note(unsigned long deltatime, unsigned short timebase, unsigned long tempo, unsigned char pitch, short velocity)
  :start(0),end(0),pitch(0),pron(""),velocity(100),is_prec(false),prec(0),is_ovrl(false),ovrl(0)
{
  this->setStart(deltatime, timebase, tempo);
  this->setPitch(pitch);
  this->setVelocity(velocity);
}

Note::Note(const Note& other)
{
  start = other.start;
  end = other.end;
  pron = other.pron;
  pitch = other.pitch;
  velocity = other.velocity;
  is_prec = other.is_prec;
  prec = other.prec;
  is_ovrl = other.is_ovrl;
  ovrl = other.ovrl;
}

Note::~Note(){}

Note& Note::operator=(const Note& other)
{
  if (this != &other) {
    start = other.start;
    end = other.end;
    pron = other.pron;
    pitch = other.pitch;
    velocity = other.velocity;
    is_prec = other.is_prec;
    prec = other.prec;
    is_ovrl = other.is_ovrl;
    ovrl = other.ovrl;
  }
  return *this;
}

bool Note::operator==(const Note& other) const
{
  bool is_eq = true;
  is_eq &= (start == other.start);
  is_eq &= (end == other.end);
  is_eq &= (pron == other.pron);
  is_eq &= (pitch == other.pitch);
  is_eq &= (velocity == other.velocity);
  is_eq &= (is_prec == other.is_prec);
  is_eq &= (prec == other.prec);
  is_eq &= (is_ovrl == other.is_ovrl);
  is_eq &= (ovrl == other.ovrl);
  return is_eq;
}

bool Note::operator!=(const Note& other) const
{
    return !(*this == other);
}

unsigned long Note::getStart()
{
  return start;
}

void Note::setStart(unsigned long start)
{
  this->start = start;
}

void Note::setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  this->start = tick2ms(deltatime, timebase, tempo);
}

unsigned long Note::getEnd()
{
  return end;
}

void Note::setEnd(unsigned long end)
{
  this->end = end;
}

void Note::setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  this->end = tick2ms(deltatime, timebase, tempo);
}

string Note::getPron()
{
  return pron;
}

void Note::setPron(string pron)
{
  this->pron = pron;
}

unsigned char Note::getPitch()
{
  return pitch;
}

double Note::getPitchHz()
{
  return 440.0 * pow(2.0,(pitch-(int)0x45)/12.0);
}

void Note::setPitch(unsigned char pitch)
{
  this->pitch = pitch;
}

unsigned short Note::getVelocity()
{
  return velocity;
}

void Note::setVelocity(short velocity)
{
  if (velocity > 0)
    this->velocity = velocity;
}

bool Note::isPrec()
{
  return is_prec;
}

unsigned short Note::getPrec()
{
  return prec;
}

void Note::setPrec(unsigned short prec)
{
  is_prec = true;
  this->prec = prec;
}

bool Note::isOvrl()
{
  return is_ovrl;
}

unsigned short Note::getOvrl()
{
  return ovrl;
}

void Note::setOvrl(unsigned short ovrl)
{
  is_ovrl = true;
  this->ovrl = ovrl;
}

double Note::tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo)
{
  return ((double)tick) / timebase * (tempo/1000.0);
}
