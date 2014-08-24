#include "Voice.h"

using namespace std;

struct Voice::Parameters Voice::params;

Voice::Voice(const Voice& other)
  :path(other.path),pron_alias(other.pron_alias),offs(other.offs),cons(other.cons),blnk(other.blnk),prec(other.prec),ovrl(other.ovrl),frq(0.0),uwc(0)
{
  if (other.hasFrq()) {
    setFrq(other.getFrq());
  }
  if (other.hasUnitWaveformContainer()) {
    setUnitWaveformContainer(other.getUnitWaveformContainer());
  }
}

Voice::~Voice()
{
  if (uwc != 0) {
    delete uwc;
    uwc = 0;
  }
}

Voice& Voice::operator=(const Voice& other)
{
  if (this != &other) {
    path = other.path;
    pron_alias = other.pron_alias;
    offs = other.offs;
    cons = other.cons;
    blnk = other.blnk;
    prec = other.prec;
    ovrl = other.ovrl;
    if (other.hasFrq()) {
      setFrq(other.getFrq());
    }
    if (other.hasUnitWaveformContainer()) {
      setUnitWaveformContainer(other.getUnitWaveformContainer());
    }
  }
  return *this;
}

bool Voice::operator==(const Voice& other) const
{
  bool is_eq = true;

  is_eq &= (path == other.path);
  is_eq &= (pron_alias == other.pron_alias);
  is_eq &= (offs == other.offs);
  is_eq &= (cons == other.cons);
  is_eq &= (blnk == other.blnk);
  is_eq &= (prec == other.prec);
  is_eq &= (ovrl == other.ovrl);

  if (hasFrq() && other.hasFrq()) {
    is_eq &= (getFrq() == other.getFrq());
  } else {
    is_eq &= (hasFrq() == other.hasFrq());
  }

  if (hasUnitWaveformContainer() && other.hasUnitWaveformContainer()) {
    is_eq &= (*getUnitWaveformContainer() == *(other.getUnitWaveformContainer()));
  } else {
    is_eq &= (hasUnitWaveformContainer() == other.hasUnitWaveformContainer());
  }

  return is_eq;
}

bool Voice::operator!=(const Voice& other) const
{
  return !(*this == other);
}

/*
 * accessor
 */
const PronunciationAlias& Voice::getPronAlias() const
{
  return pron_alias;
}

wstring Voice::getPronAliasString() const
{
  return pron_alias.prefix+pron_alias.pron+pron_alias.suffix;
}

const wstring& Voice::getPrefix() const
{
  return pron_alias.prefix;
}

const wstring& Voice::getPron() const
{
  return pron_alias.pron;
}

const wstring& Voice::getSuffix() const
{
  return pron_alias.suffix;
}

const boost::filesystem::path& Voice::getPath() const
{
  return path;
}

bool Voice::hasFrq() const
{
  return frq > 0;
}

float Voice::getFrq() const
{
  if (frq > 0.0) {
    return frq;
  }

  boost::filesystem::ifstream ifs_frq(path.parent_path()/(path.stem().wstring()+L"_wav.frq"), ios::binary);
  if (ifs_frq.fail()) {
    return 260.0;
  }
  double tmp_frq = 0.0;
  ifs_frq.seekg(sizeof(char)*12, ios_base::beg);
  ifs_frq.read((char*)&tmp_frq, sizeof(double));
  this->frq = tmp_frq;

  return this->frq;
}

void Voice::setFrq(double frq)
{
  this->frq = frq;
}

bool Voice::hasUnitWaveformContainer() const
{
  return (uwc!=0);
}

void Voice::setUnitWaveformContainer(const UnitWaveformContainer* uwc)
{
  if (this->uwc != 0) {
    delete this->uwc;
    this->uwc = 0;
  }
  *(this->uwc) = *uwc;
}

bool Voice::isVCV() const
{
  return pron_alias.checkVCV();
}

short Voice::getOffs() const
{
  return offs;
}

void Voice::setOffs(short offs)
{
  this->offs = offs;
}

short Voice::getCons() const
{
  return cons;
}

void Voice::setCons(short cons)
{
  this->cons = cons;
}

short Voice::getBlnk() const
{
  return blnk;
}

void Voice::setBlnk(short blnk)
{
  this->blnk = blnk;
}

short Voice::getPrec() const
{
  return prec;
}

void Voice::setPrec(short prec)
{
  this->prec = prec;
}

short Voice::getOvrl() const
{
  return ovrl;
}

void Voice::setOvrl(short ovrl)
{
  this->ovrl = ovrl;
}
