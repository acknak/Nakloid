#include "Note.h"

using namespace std;

Note::Note(Score *score, unsigned long id)
  :score(score),id(id)
{
  initializeNoteFrame();
}

Note::Note(Score *score, unsigned long id, unsigned long deltatime, unsigned short timebase, unsigned long tempo, unsigned char base_pitch, short base_velocity)
  :score(score),id(id)
{
  initializeNoteFrame();
  this->setStart(deltatime, timebase, tempo);
  this->setBasePitch(base_pitch);
  self.base_velocity = base_velocity;
}

Note::Note(const Note& other)
{
  score = other.score;
  id = other.id;
  self = other.self;
  if (other.self.prec != 0)
    self.prec = new short (*(other.self.prec));
  if (other.self.ovrl != 0)
    self.ovrl = new short (*(other.self.ovrl));
}

Note::~Note()
{
  if (self.ovrl != 0) {
    delete self.ovrl;
    self.ovrl = 0;
  }
  if (self.prec != 0) {
    delete self.prec;
    self.prec = 0;
  }
}

Note& Note::operator=(const Note& other)
{
  if (this != &other) {
    score = other.score;
    id = other.id;
    self = other.self;
    if (other.self.prec != 0)
      self.prec = new short (*(other.self.prec));
    if (other.self.ovrl != 0)
      self.ovrl = new short (*(other.self.ovrl));
  }
  return *this;
}

bool Note::operator==(const Note& other) const
{
  bool is_eq = true;
  is_eq &= (score == other.score);
  is_eq &= (id == other.id);
  is_eq &= (self.start == other.self.start);
  is_eq &= (self.end == other.self.end);
  is_eq &= (self.pron == other.self.pron);
  is_eq &= (self.base_pitch == other.self.base_pitch);
  is_eq &= (self.base_velocity == other.self.base_velocity);
  is_eq &= (self.velocities == other.self.velocities);
  is_eq &= (this->getPrec() == other.getPrec());
  is_eq &= (this->getOvrl() == other.getOvrl());
  return is_eq;
}

bool Note::operator!=(const Note& other) const
{
    return !(*this == other);
}

unsigned long Note::getStart()
{
  return self.start;
}

unsigned long Note::getPronStart()
{
  long tmp = self.start - (getPrec()-((getOvrl()<0)?getOvrl():0));
  return tmp>0?tmp:0;
}

void Note::setStart(unsigned long ms_start)
{
  this->self.start = ms_start;
}

void Note::setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  setStart(tick2ms(deltatime, timebase, tempo));
}

unsigned long Note::getEnd()
{
  return self.end;
}

unsigned long Note::getPronEnd()
{
  long tmp = self.end - getLack();
  if (getPronStart() > tmp) {
    cerr << "[Note::getPronEnd] pron_start > pron_end" << endl;
    return getPronStart();
  }
  return tmp>0?tmp:0;
}

void Note::setEnd(unsigned long ms_end)
{
  self.end = ms_end;
}

void Note::setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  setEnd(tick2ms(deltatime, timebase, tempo));
}

string Note::getPron()
{
  return self.pron;
}

void Note::setPron(string pron)
{
  this->self.pron = pron;
}

unsigned char Note::getBasePitch()
{
  return self.base_pitch;
}

double Note::getBasePitchHz()
{
  return 440.0 * pow(2.0,(self.base_pitch-(int)0x45)/12.0);
}

void Note::setBasePitch(unsigned char base_pitch)
{
  this->self.base_pitch = base_pitch;
}

short Note::getBaseVelocity()
{
  return self.base_velocity;
}

void Note::setBaseVelocity(short base_velocity)
{
  self.base_velocity = base_velocity;
}

void Note::addVelocityPoint(long ms, short vel)
{
  self.velocities.push_back(make_pair(ms, vel));
}

short Note::getVelocityPointNum()
{
  return self.velocities.size();
}

vector<short> Note::getVelocities()
{
  long velocities_size = getPronEnd()-getPronStart();
  vector<short> velocities(velocities_size, 0);

  map<long,short> tmp_vels;
  for (list<pair<long,short>>::iterator it=self.velocities.begin(); it!=self.velocities.end(); ++it) {
    long tmp_ms = (it->first)<0?velocities_size+it->first:it->first;
    if (tmp_ms < velocities_size && tmp_ms > 0)
      tmp_vels[tmp_ms] = it->second;
  }

  if (tmp_vels.size() == 0)
    velocities.assign(velocities_size, self.base_velocity);
  else {
    if (tmp_vels.find(0) == tmp_vels.end())
      tmp_vels[0] = 0;
    if (tmp_vels.find(velocities_size-1) == tmp_vels.end())
      tmp_vels[velocities_size-1] = 0;
    for (map<long,short>::iterator it=++tmp_vels.begin(); it!=tmp_vels.end(); ++it)
      for (int i=0; i<it->first-boost::prior(it)->first; i++)
        velocities[i+boost::prior(it)->first] = 
          (1.0/(it->first-boost::prior(it)->first)*i*(it->second-boost::prior(it)->second)+boost::prior(it)->second)
          *self.base_velocity/100.0;
  }

  return velocities;
}

short Note::getLack()
{
  return score->getNoteLack(this);
}

bool Note::isPrec()
{
  return self.prec!=0;
}

short Note::getPrec() const
{
  return (self.prec==0)?0:*self.prec;
}

void Note::setPrec(short prec)
{
  if (self.prec != 0) {
    delete self.prec;
    self.prec = 0;
  }
  self.prec = new short (prec);
}

bool Note::isOvrl()
{
  return self.ovrl!=0;
}

short Note::getOvrl() const
{
  return (self.ovrl==0)?0:*self.ovrl;
}

void Note::setOvrl(short ovrl)
{
  if (self.ovrl != 0) {
    delete self.ovrl;
    self.ovrl = 0;
  }
  self.ovrl = new short (ovrl);
}

void Note::initializeNoteFrame()
{
  self.start = 0;
  self.end = 0;
  self.pron = "";
  self.base_pitch = 0x45;
  self.base_velocity = 100;
  self.prec = 0;
  self.ovrl = 0;
}

unsigned long Note::tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo)
{
  return (unsigned long)(((double)tick) / timebase * (tempo/1000.0));
}
