#include "Note.h"

using namespace std;

Note::Note()
  :start(0),end(0),base_pitch(0),base_velocity(100),pron(""),is_prec(false),prec(0),is_ovrl(false),ovrl(0){}

Note::Note(unsigned long start, unsigned char base_pitch, short velocity)
  :start(0),end(0),base_pitch(0),base_velocity(100),pron(""),is_prec(false),prec(0),is_ovrl(false),ovrl(0)
{
  this->setStart(start);
  this->setBasePitch(base_pitch);
  this->setVelocity(velocity);
}

Note::Note(unsigned long deltatime, unsigned short timebase, unsigned long tempo, unsigned char base_pitch, short velocity)
  :start(0),end(0),base_pitch(0),pron(""),is_prec(false),prec(0),is_ovrl(false),ovrl(0)
{
  this->setStart(deltatime, timebase, tempo);
  this->setBasePitch(base_pitch);
  this->setVelocity(velocity);
}

Note::Note(const Note& other)
{
  start = other.start;
  end = other.end;
  pron = other.pron;
  base_pitch = other.base_pitch;
  base_velocity = other.base_velocity;
  velocities = other.velocities;
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
    base_pitch = other.base_pitch;
    base_velocity = other.base_velocity;
    velocities = other.velocities;
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
  is_eq &= (base_pitch == other.base_pitch);
  is_eq &= (base_velocity == other.base_velocity);
  is_eq &= (velocities == other.velocities);
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
  if (!velocities.empty())
    if (this->start < start)
      velocities.insert(velocities.begin(), start-this->start, velocities.front());
    else
      velocities.erase(velocities.begin(), velocities.begin()+(start-this->start));
  this->start = start;
}

void Note::setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  unsigned long tmp_start = tick2ms(deltatime, timebase, tempo);
  if (!velocities.empty())
    if (this->start < tmp_start)
      velocities.insert(velocities.begin(), tmp_start-this->start, velocities.front());
    else
      velocities.erase(velocities.begin(), velocities.begin()+(tmp_start-this->start));
  this->start = tmp_start;
}

unsigned long Note::getEnd()
{
  return end;
}

void Note::setEnd(unsigned long end)
{
  if (!velocities.empty())
    if (this->end < end)
      velocities.insert(velocities.end(), end-this->end, velocities.back());
    else
      velocities.erase(velocities.end()-(end-this->end), velocities.end());
  this->end = end;
}

void Note::setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  unsigned long tmp_end = tick2ms(deltatime, timebase, tempo);
  if (!velocities.empty())
    if (this->end < tmp_end)
      velocities.insert(velocities.end(), tmp_end-this->end, velocities.back());
    else
      velocities.erase(velocities.end()-(tmp_end-this->end), velocities.end());
  this->end = tmp_end;
}

string Note::getPron()
{
  return pron;
}

void Note::setPron(string pron)
{
  this->pron = pron;
}

unsigned char Note::getBasePitch()
{
  return base_pitch;
}

double Note::getBasePitchHz()
{
  return 440.0 * pow(2.0,(base_pitch-(int)0x45)/12.0);
}

void Note::setBasePitch(unsigned char base_pitch)
{
  this->base_pitch = base_pitch;
}

vector<unsigned char> Note::getVelocities()
{
  if (start >= end)
    cerr << "[Note::getVelocities()] Note on time not defined" << endl;
  else if (velocities.empty() && start<end)
    velocities.assign(end-start, base_velocity);
  return velocities;
}

void Note::setVelocity(unsigned char velocity)
{
  base_velocity = velocity;
}

void Note::setVelocities(vector<unsigned char> velocities)
{
  if (velocities.size() == end-start)
    this->velocities = velocities;
  else
    cerr << "[Note::setVelocities()] Velocity length differ from Note on time" << endl;
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
