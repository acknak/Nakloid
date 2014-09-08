#include "Note.h"

using namespace std;

struct Note::Parameters Note::params;

Note::Note(Score *score, long id)
  :score(score), id(id), self(){}

Note::Note(Score *score, long id, long ms_start, unsigned char base_pitch, short base_velocity)
  :score(score),id(id),self()
{
  this->setStart(ms_start);
  this->setBasePitch(base_pitch);
  self.base_velocity = base_velocity;
}

Note::Note(const Note& other)
{
  score = other.score;
  id = other.id;
  self = other.self;
  if (other.self.margin != 0) {
    self.margin = new pair<short,short>(*(other.self.margin));
  }
  if (other.self.padding != 0) {
    self.padding = new pair<short,short>(*(other.self.padding));
  }
  if (other.self.prec != 0) {
    self.prec = new short (*(other.self.prec));
  }
  if (other.self.ovrl != 0) {
    self.ovrl = new short (*(other.self.ovrl));
  }
  if (other.self.cons != 0) {
    self.cons = new short (*(other.self.cons));
  }
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
    if (other.self.margin != 0) {
      self.margin = new pair<short,short>(*(other.self.margin));
    }
    if (other.self.padding != 0) {
      self.padding = new pair<short,short>(*(other.self.padding));
    }
    if (other.self.prec != 0) {
      self.prec = new short (*(other.self.prec));
    }
    if (other.self.ovrl != 0) {
      self.ovrl = new short (*(other.self.ovrl));
    }
    if (other.self.cons != 0) {
      self.cons = new short (*(other.self.cons));
    }
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
  is_eq &= (self.pron_alias.prefix == other.self.pron_alias.prefix);
  is_eq &= (self.pron_alias.pron == other.self.pron_alias.pron);
  is_eq &= (self.pron_alias.suffix == other.self.pron_alias.suffix);
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
  return self.start - getPrec() - ((getOvrl()<0)?getOvrl():0) + score->getMargin() + getFrontMargin();
}

void Note::setStart(long ms_start)
{
  this->self.start = ms_start;
}

long Note::getEnd() const
{
  return self.end;
}

long Note::getPronEnd() const
{
  long tmp_pron_end = self.end + score->getMargin() - getBackMargin();
  if (getPronStart() > tmp_pron_end) {
    cerr << "[Note::getPronEnd] pron_start > pron_end" << endl;
    return getPronStart()+1;
  }
  return tmp_pron_end;
}

void Note::setEnd(long ms_end)
{
  self.end = ms_end;
}

short Note::getFrontMargin() const
{
  if (self.margin != 0) {
    return self.margin->first;
  }

  const Note* note_prev = score->getPrevNote(this);
  if (note_prev == 0) {
    return 0;
  }

  if (isVCV()) {
    long ms_prev_cons_start = note_prev->getStart() - note_prev->getPrec() + note_prev->getCons();
    long ms_start = getStart() - getPrec();
    if (ms_start < ms_prev_cons_start) {
      long tmp_margin = ms_prev_cons_start - ms_start;
      long tmp_padding = max(params.ms_front_padding, params.ms_back_padding);
      tmp_padding = min(tmp_padding, (long)getOvrl());
      if (getPrec()-tmp_margin > tmp_padding) {
        return tmp_margin;
      } else {
        return getPrec() - tmp_padding;
      }
    }
  }
  return 0;
}

short Note::getBackMargin() const
{
  if (self.margin != 0) {
    return self.margin->second;
  }

  const Note* note_next = score->getNextNote(this);
  if (note_next==0 || getEnd() < note_next->getStart()){
    return 0;
  }

  if (isVCV() || note_next->isVCV()) {
    return getEnd()+score->getMargin() - (note_next->getPronStart()+note_next->getFrontPadding());
  }
  return note_next->getPrec() - note_next->getOvrl();
}

void Note::setMargin(short front, short back)
{
  if (self.margin != 0) {
    delete self.margin;
    self.margin = 0;
  }
  self.margin = new pair<short,short>(front, back);
}

short Note::getFrontPadding() const
{
  if (self.padding != 0) {
    return self.padding->first;
  }

  if (isVCV()) {
    long tmp_padding = getOvrl()-getFrontMargin(), default_padding = max(params.ms_front_padding, params.ms_back_padding);
    return max(tmp_padding, default_padding);
  }
  return params.ms_front_padding;
}

short Note::getBackPadding() const
{
  if (self.padding != 0) {
    return self.padding->second;
  }

  const Note* note_next = score->getNextNote(this);
  if (note_next!=0 && note_next->isVCV()) {
    if (note_next!=0) {
      return note_next->getFrontPadding();
    }
  }
  return max(params.ms_front_padding, params.ms_back_padding);
}

void Note::setPadding(short front, short back)
{
  if (self.padding != 0) {
    delete self.padding;
    self.padding = 0;
  }
  self.padding = new pair<short,short>(front, back);
}

PronunciationAlias Note::getPronAlias() const
{
  return self.pron_alias;
}

wstring Note::getPronAliasString() const
{
  return self.pron_alias.getAliasString();
}

void Note::setPronAlias(PronunciationAlias voice_alias)
{
  self.pron_alias = voice_alias;
}

void Note::setPronAlias(const std::wstring& alias)
{
  PronunciationAlias voice_alias(alias);
  setPronAlias(voice_alias);
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

void Note::setVelocityPoints(const vector< pair<long,short> > &velocity_points)
{
  self.velocity_points = velocity_points;
}

const vector< pair<long,short> >& Note::getVelocityPoints() const
{
  return self.velocity_points;
}

short Note::getVelocityPointNum() const
{
  return self.velocity_points.size();
}

vector<short> Note::getVelocities() const
{
  long ms_pron_start=getPronStart(), ms_pron_end=getPronEnd();
  long velocities_size = ms_pron_end-((getPronStart()>0)?ms_pron_start:0);
  vector<short> velocities(velocities_size, 100*(params.auto_vowel_combining?params.vowel_combining_volume:1.0));
  vector< pair<long,short> > tmp_velocity_points = getVelocityPoints();

  if (tmp_velocity_points.size() > 0) {
    // sanitize
    map<long,short> tmp_vels;
    for (vector< pair<long,short> >::iterator it=tmp_velocity_points.begin(); it!=tmp_velocity_points.end(); ++it) {
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
  }

  // padding
  short padding_front=getFrontPadding(), padding_back=getBackPadding();
  for (size_t i=0; i<padding_front; i++) {
    velocities[i] *= i / (double)padding_front;
  }
  for (size_t i=0; i<padding_back; i++) {
    velocities[velocities.size()-1-i] *= i / (double)padding_back;
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
