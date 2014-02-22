#include "Voice.h"

using namespace std;
using namespace uw;

Voice::Voice():voice_db(0),path_wav(L""),alias(),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),uwc(0){}

Voice::Voice(VoiceDB* const voice_db):voice_db(voice_db),path_wav(L""),alias(),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),uwc(0){}

Voice::Voice(const Voice& other):voice_db(0),path_wav(L""),alias(),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),uwc(0)
{
  voice_db = other.voice_db;
  path_wav = other.path_wav;
  alias = other.alias;
  is_vcv = other.is_vcv;
  offs = other.offs;
  cons = other.cons;
  blnk = other.blnk;
  prec = other.prec;
  ovrl = other.ovrl;

  if (other.hasFrq()) {
    setFrq(other.getFrq());
  }

  if (other.hasUwc()) {
    setUwc(*other.getUwc());
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
    voice_db = other.voice_db;
    path_wav = other.path_wav;
    alias = other.alias;
    is_vcv = other.is_vcv;
    offs = other.offs;
    cons = other.cons;
    blnk = other.blnk;
    prec = other.prec;
    ovrl = other.ovrl;
    if (other.hasFrq()) {
      setFrq(other.getFrq());
    }
    if (other.hasUwc()) {
      setUwc(*other.getUwc());
    }
  }
  return *this;
}

bool Voice::operator==(const Voice& other) const
{
  bool is_eq = true;

  is_eq &= (path_wav == other.path_wav);
  is_eq &= (alias.pron == other.alias.pron);
  is_eq &= (alias.prefix == other.alias.prefix);
  is_eq &= (alias.suffix == other.alias.suffix);
  is_eq &= (is_vcv == other.is_vcv);
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

  if (hasUwc() && other.hasUwc()) {
    is_eq &= (getUwc() == other.getUwc());
  } else {
    is_eq &= (hasUwc() == other.hasUwc());
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
const nak::VoiceAlias& Voice::getAlias() const
{
  return alias;
}

wstring Voice::getAliasString() const
{
  return alias.prefix+alias.pron+alias.suffix;
}

const wstring& Voice::getPrefix() const
{
  return alias.prefix;
}

const wstring& Voice::getPron() const
{
  return alias.pron;
}

const wstring& Voice::getSuffix() const
{
  return alias.suffix;
}

void Voice::setAlias(const std::wstring &alias)
{
  nak::VoiceAlias voice_alias(alias);
  this->alias = voice_alias;
}

void Voice::setWavPath(const wstring &path_wav)
{
  this->path_wav = path_wav;
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

  boost::filesystem::ifstream ifs_frq((path_wav.parent_path()/path_wav.stem()).wstring()+L"_wav.frq", ios::binary);
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

bool Voice::hasUwc() const
{
  return (uwc!=0);
}

const UnitWaveformContainer* Voice::getUwc() const
{
  if (uwc != 0) {
    return uwc;
  }

  this->uwc = new UnitWaveformContainer;
  wstring alias_string = getAliasString();
  boost::filesystem::path path_uwc((path_wav.parent_path()/boost::algorithm::replace_all_copy(alias_string, L"*", L"_")).wstring()+L".uwc");

  wcout << L"load voice \"" << alias_string << L"\" from ";

  if (nak::cache && uw::isUwcFile(path_uwc.wstring())) {
    cout << "cache" << endl;
    *uwc = uw::load(path_uwc.wstring());
    return uwc;
  }

  // get wav data
  vector<double> wav_data;
  long wav_fs = 0;
  {
    cout << "wav data" << endl;
    WavParser wav_parser(path_wav.wstring());
    wav_parser.addTargetTrack(0);
    if (!wav_parser.parse()) {
      return 0;
    }
    wav_data = (*(wav_parser.getDataChunks().begin())).getData();
    wav_fs = wav_parser.getFormat().dwSamplesPerSec;
  }

  // make input pitch mark
  vector<long> input_pitchmarks;
  {
    PitchMarker *marker = new PitchMarker(wav_data, offs, ovrl, prec, blnk, wav_fs);
    if (voice_db->isVowel(alias.getVowel())) {
      short win_size = wav_fs / getFrq() * 2;
      vector<double> aft_vowel_wav = voice_db->getVowel(alias.getVowel());
      trimVector(&aft_vowel_wav, win_size);
      if (is_vcv && voice_db->isVowel(alias.getPrefixVowel())) {
        vector<double> fore_vowel_wav = voice_db->getVowel(alias.getPrefixVowel());
        trimVector(&fore_vowel_wav, win_size);
        marker->mark(fore_vowel_wav, aft_vowel_wav);
      } else {
        marker->mark(aft_vowel_wav);
      }
    } else {
      marker->mark(getFrq(), wav_fs);
    }
    input_pitchmarks = marker->getPitchMarks();
    delete marker;
  }

  // make unit waveforms
  {
    UnitWaveformMaker *maker = new UnitWaveformMaker();
    maker->setPitchMarks(input_pitchmarks, offs+cons, offs+ovrl, wav_fs);
    maker->makeUnitWaveform(wav_data, wav_fs/getFrq(), is_vcv);
    uwc->unit_waveforms = maker->getUnitWaveform();
    uwc->format.wLobeSize = nak::unit_waveform_lobe;
    uwc->format.dwRepeatStart = maker->getFadeStartSub();
    uwc->format.wF0 = getFrq();
    delete maker;
  }

  // output uwc
  if (nak::cache) {
    uwc->format.setDefaultValues();
    uwc->format.wFormatTag = UnitWaveformFormat::UnitWaveformFormatTag;
    uwc->format.dwSamplesPerSec = wav_fs;
    uw::save(path_uwc.wstring(), *uwc);
  }

  return this->uwc;
}

void Voice::setUwc(const UnitWaveformContainer& uwc)
{
  if (this->uwc != 0) {
    delete this->uwc;
    this->uwc = 0;
  }
  *(this->uwc) = uwc;
}

/*
 * protected
 */
template <class Vector>
Vector* Voice::trimVector(Vector* target_vector, long target_length) const
{
  if (target_vector->size() > target_length) {
    short space = target_vector->size() - target_length;
    target_vector->erase(target_vector->begin(), target_vector->begin()+space/2);
    target_vector->erase(target_vector->end()-(space-(space/2)),target_vector->end());
  } else if (target_vector->size() < target_length) {
    short space = target_length - target_vector->size();
    target_vector->insert(target_vector->begin(), space/2, 0);
    target_vector->insert(target_vector->end(), space-(space/2), 0);
  }

  return target_vector;
}
