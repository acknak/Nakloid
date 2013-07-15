#include "Note.h"

using namespace std;

Note::Note(Score *score, long id):score(score),id(id),self(){}

Note::Note(Score *score, long id, unsigned long deltatime, unsigned short timebase, unsigned long tempo, unsigned char base_pitch, short base_velocity)
  :score(score),id(id),self()
{
  this->setStart(deltatime, timebase, tempo);
  this->setBasePitch(base_pitch);
  self.base_velocity = base_velocity;
}

Note::Note(const Note& other)
{
  score = other.score;
  id = other.id;
  self = other.self;
  if (other.self.margin != 0)
    self.margin = new pair<short,short>(*(other.self.margin));
  if (other.self.padding != 0)
    self.padding = new pair<short,short>(*(other.self.padding));
  if (other.self.prec != 0)
    self.prec = new short (*(other.self.prec));
  if (other.self.ovrl != 0)
    self.ovrl = new short (*(other.self.ovrl));
  if (other.self.cons != 0)
    self.cons = new short (*(other.self.cons));
}

Note::~Note()
{
  if (self.margin != 0) {
    delete self.margin;
    self.margin = 0;
  }
  if (self.padding != 0) {
    delete self.padding;
    self.padding = 0;
  }
  if (self.prec != 0) {
    delete self.prec;
    self.prec = 0;
  }
  if (self.ovrl != 0) {
    delete self.ovrl;
    self.ovrl = 0;
  }
  if (self.cons != 0) {
    delete self.cons;
    self.cons = 0;
  }
}

Note& Note::operator=(const Note& other)
{
  if (this != &other) {
    score = other.score;
    id = other.id;
    self = other.self;
    if (other.self.margin != 0)
      self.margin = new pair<short,short>(*(other.self.margin));
    if (other.self.padding != 0)
      self.padding = new pair<short,short>(*(other.self.padding));
    if (other.self.prec != 0)
      self.prec = new short (*(other.self.prec));
    if (other.self.ovrl != 0)
      self.ovrl = new short (*(other.self.ovrl));
    if (other.self.cons != 0)
      self.cons = new short (*(other.self.cons));
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
  is_eq &= (self.alias.prefix == other.self.alias.prefix);
  is_eq &= (self.alias.pron == other.self.alias.pron);
  is_eq &= (self.alias.suffix == other.self.alias.suffix);
  is_eq &= (self.base_pitch == other.self.base_pitch);
  is_eq &= (self.base_velocity == other.self.base_velocity);
  is_eq &= (self.velocity_points == other.self.velocity_points);
  if (self.margin!=other.self.margin && self.margin!=0 && other.self.margin!=0) {
    is_eq &= (*self.margin!=*other.self.margin);
  }
  if (self.padding!=other.self.padding && self.padding!=0 && other.self.padding!=0) {
    is_eq &= (*self.padding!=*other.self.padding);
  }
  is_eq &= (this->getPrec() == other.getPrec());
  is_eq &= (this->getOvrl() == other.getOvrl());
  is_eq &= (this->getCons() == other.getCons());
  is_eq &= (self.is_vcv == other.self.is_vcv);
  return is_eq;
}

bool Note::operator!=(const Note& other) const
{
    return !(*this == other);
}

/*
 * accessor
 */
long Note::getId() const
{
  return id;
}

long Note::getStart() const
{
  return self.start;
}

long Note::getPronStart() const
{
  long pron_start = self.start - getPrec();
  if (getOvrl() < 0) {
    pron_start -= getOvrl();
  }
  return (pron_start>0)?pron_start:0;
}

void Note::setStart(long ms_start)
{
  this->self.start = ms_start;
}

void Note::setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  setStart(nak::tick2ms(deltatime, timebase, tempo));
}

long Note::getEnd() const
{
  return self.end;
}

long Note::getPronEnd() const
{
  if (getPronStart() > self.end) {
    cerr << "[Note::getPronEnd] pron_start > pron_end" << endl;
    return getPronStart();
  }
  return self.end;
}

void Note::setEnd(long ms_end)
{
  self.end = ms_end;
}

void Note::setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo)
{
  setEnd(nak::tick2ms(deltatime, timebase, tempo));
}

short Note::getFrontMargin()
{
  if (self.margin != 0) {
    return self.margin->first;
  }

  Note* note_prev = score->getPrevNote(this);
  if (note_prev == 0) {
    return 0;
  }

  if (isVCV()) {
    if (note_prev->getPronEnd()-note_prev->getBackMargin() > getPronStart()+getOvrl()) {
      return note_prev->getPronEnd() - note_prev->getBackMargin() - nak::ms_back_padding - getPronStart();
    }
  }
  return 0;
}

short Note::getBackMargin()
{
  if (self.margin != 0) {
    return self.margin->second;
  }

  Note* note_next = score->getNextNote(this);
  if (note_next == 0){
    return 0;
  }

  if (note_next->isVCV()) {
    if (getPronStart()+getCons()+nak::ms_back_padding < note_next->getPronStart()+note_next->getOvrl()) {
      return note_next->getPrec() - note_next->getOvrl();
    } else {
      return getPronEnd() - (getPronStart()+getCons()+nak::ms_back_padding);
    }
  }
  return 0;
}

void Note::setMargin(short front, short back)
{
  if (self.margin != 0) {
    delete self.margin;
    self.margin = 0;
  }
  self.margin = new pair<short,short>(front, back);
}

short Note::getFrontPadding()
{
  if (self.padding != 0) {
    return self.padding->first;
  }

  if (isVCV()) {
    if (getOvrl()-getFrontMargin() > nak::ms_back_padding) {
      return getOvrl() - getFrontMargin();
    } else {
      return nak::ms_back_padding;
    }
  }
  return nak::ms_front_padding;
}

short Note::getBackPadding()
{
  if (self.padding != 0) {
    return self.padding->second;
  }

  Note* note_next = score->getNextNote(this);
  if (note_next!=0 && note_next->isVCV()) {
    if (note_next!=0) {
      return note_next->getFrontPadding();
    }
  }
  return nak::ms_back_padding;
}

void Note::setPadding(short front, short back)
{
  if (self.padding != 0) {
    delete self.padding;
    self.padding = 0;
  }
  self.padding = new pair<short,short>(front, back);
}

const wstring& Note::getPron() const
{
  return self.alias.pron;
}

void Note::setPron(const wstring& pron)
{
  this->self.alias.pron = pron;
}

const wstring& Note::getPrefix() const
{
  return self.alias.prefix;
}

void Note::setPrefix(const wstring& prefix)
{
  this->self.alias.prefix = prefix;
}

const wstring& Note::getSuffix() const
{
  return self.alias.suffix;
}

void Note::setSuffix(const wstring& suffix)
{
  this->self.alias.suffix = suffix;
}

nak::VoiceAlias Note::getAlias() const
{
  return self.alias;
}

wstring Note::getAliasString() const
{
  return self.alias.prefix+self.alias.pron+self.alias.suffix;
}

void Note::setAlias(nak::VoiceAlias voice_alias)
{
  self.alias = voice_alias;
}

void Note::setAlias(const std::wstring& alias)
{
  nak::VoiceAlias voice_alias(alias);
  self.alias = voice_alias;
}

unsigned char Note::getBasePitch() const
{
  return self.base_pitch;
}

float Note::getBasePitchHz() const
{
  return 440.0 * pow(2.0,(self.base_pitch-(int)0x45)/12.0);
}

void Note::setBasePitch(unsigned char base_pitch)
{
  this->self.base_pitch = base_pitch;
}

short Note::getBaseVelocity() const
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

void Note::setVelocityPoints(const list< pair<long,short> >& velocity_points)
{
  self.velocity_points = velocity_points;
}

list< pair<long,short> > Note::getVelocityPoints()
{
  if (self.velocity_points.size() > 0) {
    return self.velocity_points;
  }

  // return default points
  list < pair<long,short> > tmp_velocities;
  short margin_front=getFrontMargin(), margin_back=getBackMargin(), padding_front=getFrontPadding(), padding_back=getBackPadding();
  tmp_velocities.push_back(make_pair(0, 0));
  tmp_velocities.push_back(make_pair(margin_front, 0));
  tmp_velocities.push_back(make_pair(margin_front+padding_front, self.base_velocity));
  tmp_velocities.push_back(make_pair(-margin_back-padding_back, self.base_velocity));
  tmp_velocities.push_back(make_pair(-margin_back, 0));
  tmp_velocities.push_back(make_pair(-1, 0));
  return tmp_velocities;
}

short Note::getVelocityPointNum() const
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
    for (size_t i=0; i<it->first-boost::prior(it)->first; i++) {
      velocities[i+boost::prior(it)->first] =
        (1.0/(it->first-boost::prior(it)->first)*i*(it->second-boost::prior(it)->second)+boost::prior(it)->second)*(self.base_velocity/100.0);
    }
  }

  return velocities;
}

bool Note::isPrec() const
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

bool Note::isOvrl() const
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

bool Note::isCons() const
{
  return self.cons!=0;
}

short Note::getCons() const
{
  return (self.cons==0)?0:*self.cons;
}

void Note::setCons(short cons)
{
  if (self.cons != 0) {
    delete self.cons;
    self.cons = 0;
  }
  self.cons = new short (cons);
}

bool Note::isVCV() const
{
  return self.is_vcv;
}

void Note::isVCV(bool is_vcv)
{
  self.is_vcv = is_vcv;
}
