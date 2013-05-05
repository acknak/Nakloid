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
  is_eq &= (self.prefix == other.self.prefix);
  is_eq &= (self.suffix == other.self.suffix);
  is_eq &= (self.base_pitch == other.self.base_pitch);
  is_eq &= (self.base_velocity == other.self.base_velocity);
  is_eq &= (self.velocity_points == other.self.velocity_points);
  is_eq &= (this->getPrec() == other.getPrec());
  is_eq &= (this->getOvrl() == other.getOvrl());
  is_eq &= (self.is_vcv == other.self.is_vcv);
  is_eq &= (self.is_cv_proxy == other.self.is_cv_proxy);
  return is_eq;
}

bool Note::operator!=(const Note& other) const
{
    return !(*this == other);
}

unsigned long Note::getId()
{
  return id;
}

long Note::getStart()
{
  return self.start;
}

long Note::getPronStart()
{
  return self.start - getPrec();
}

void Note::setStart(unsigned long ms_start)
{
  this->self.start = ms_start;
}

void Note::setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  setStart(nak::tick2ms(deltatime, timebase, tempo));
}

long Note::getEnd()
{
  return self.end;
}

long Note::getPronEnd()
{
  long tmp = self.end - (score->getNextNoteDist(this)>0?0:getLack());
  if (getPronStart() > tmp) {
    cerr << "[Note::getPronEnd] pron_start > pron_end" << endl;
    return getPronStart();
  }
  return tmp;
}

void Note::setEnd(unsigned long ms_end)
{
  self.end = ms_end;
}

void Note::setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  setEnd(nak::tick2ms(deltatime, timebase, tempo));
}

string Note::getPron()
{
  return self.pron;
}

void Note::setPron(string pron)
{
  this->self.pron = pron;
  this->self.is_vcv = (pron.find(" ")!=string::npos && (pron[0]!='*'&&pron[0]!='-'));
}

string Note::getPrefix()
{
  return self.prefix;
}

void Note::setPrefix(string prefix)
{
  this->self.prefix = prefix;
}

string Note::getSuffix()
{
  return self.suffix;
}

void Note::setSuffix(string suffix)
{
  this->self.suffix = suffix;
}

string Note::getAlias()
{
  return self.prefix+self.pron+self.suffix;
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
  self.velocity_points.push_back(make_pair(ms, vel));
}

list< pair<long,short> > Note::getVelocityPoints()
{
  if (self.velocity_points.size() > 0) {
    return self.velocity_points;
  }

  // return default points
  list < pair<long,short> > tmp_velocities;
  tmp_velocities.push_back(make_pair(0, 0));
  if (self.is_vcv && self.ovrl!=0) {
    tmp_velocities.push_back(make_pair(*self.ovrl, self.base_velocity));
  } else {
    tmp_velocities.push_back(make_pair(nak::ms_front_edge, self.base_velocity));
  }
  if (score->isNextNoteVCV(this)) {
    tmp_velocities.push_back(make_pair(-score->getNextNoteOvrl(this), self.base_velocity));
  } else {
    tmp_velocities.push_back(make_pair(-nak::ms_back_edge, self.base_velocity));
  }
  tmp_velocities.push_back(make_pair(-1, 0));
  return tmp_velocities;
}

short Note::getVelocityPointNum()
{
  return self.velocity_points.size();
}

vector<short> Note::getVelocities()
{
  long velocities_size = getPronEnd()-getPronStart();
  vector<short> velocities(velocities_size, 0);
  list< pair<long,short> > tmp_velocity_points = getVelocityPoints();

  // sanitize
  map<long,short> tmp_vels;
  for (list< pair<long,short> >::iterator it=tmp_velocity_points.begin(); it!=tmp_velocity_points.end(); ++it) {
    long tmp_ms = (it->first)<0?velocities_size+it->first:it->first;
    if (tmp_ms < velocities_size && tmp_ms >= 0)
      tmp_vels[tmp_ms] = it->second;
  }

  // vels to velocities
  for (map<long,short>::iterator it=++tmp_vels.begin(); it!=tmp_vels.end(); ++it) {
    for (int i=0; i<it->first-boost::prior(it)->first; i++) {
      velocities[i+boost::prior(it)->first] =
        (1.0/(it->first-boost::prior(it)->first)*i*(it->second-boost::prior(it)->second)+boost::prior(it)->second)
        *self.base_velocity/100.0;
    }
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

bool Note::isVCV()
{
  return self.is_vcv;
}

void Note::isVCV(bool is_vcv)
{
  self.is_vcv = is_vcv;
}

bool Note::isCVProxy()
{
  return self.is_cv_proxy;
}

void Note::isCVProxy(bool is_cv_proxy)
{
  self.is_cv_proxy = is_cv_proxy;
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
  self.is_vcv = false;
  self.is_cv_proxy = false;
}
