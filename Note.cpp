#include "Note.h"

using namespace std;

Note::Note()
{
  start = length = pitch = 0;
  pron = "";
  velocity = 100;
  prec = ovrl = -1;
}

Note::Note(double start, unsigned char pitch, short velocity)
{
  Note();
  this->setStart(start);
  this->setPitch(pitch);
  this->setVelocity(velocity);
}

Note::Note(const Note& other)
{
  start = other.start;
  length = other.length;
  pron = other.pron;
  pitch = other.pitch;
  velocity = other.velocity;
  prec = other.prec;
  ovrl = other.ovrl;
}

Note::~Note(){}

Note& Note::operator=(const Note& other)
{
  if (this != &other) {
    start = other.start;
    length = other.length;
    pron = other.pron;
    pitch = other.pitch;
    velocity = other.velocity;
    prec = other.prec;
    ovrl = other.ovrl;
  }
  return *this;
}

bool Note::operator==(const Note& other) const
{
  bool is_eq = true;
  is_eq &= (start == other.start);
  is_eq &= (length == other.length);
  is_eq &= (pron == other.pron);
  is_eq &= (pitch == other.pitch);
  is_eq &= (velocity == other.velocity);
  is_eq &= (prec == other.prec);
  is_eq &= (ovrl == other.ovrl);
  return is_eq;
}

bool Note::operator!=(const Note& other) const
{
    return !(*this == other);
}

double Note::getStart()
{
  return start;
}

void Note::setStart(double start)
{
  this->start = start;
}

void Note::setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  this->start = tick2ms(deltatime, timebase, tempo);
}

double Note::getLength()
{
  return length;
}

void Note::setLength(double length)
{
  this->length = length;
}

void Note::setLength(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  this->length = tick2ms(deltatime, timebase, tempo);
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

short Note::getPrec()
{
  return prec;
}

void Note::setPrec(short prec)
{
  if (prec > 0)
    this->prec = prec;
}

short Note::getOvrl()
{
  return ovrl;
}

void Note::setOvrl(short ovrl)
{
  if (ovrl > 0)
    this->ovrl = ovrl;
}

double Note::tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo)
{
  return ((double)tick) / timebase * (tempo/1000.0);
}
