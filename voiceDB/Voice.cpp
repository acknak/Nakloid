#include "Voice.h"

using namespace std;

Voice::Voice():voice_db(0),path_wav(""),pron(""),prefix(""),suffix(""),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),bwc(0){}

Voice::Voice(VoiceDB* voice_db):voice_db(voice_db),path_wav(""),pron(""),prefix(""),suffix(""),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),bwc(0){}

Voice::Voice(const Voice& other):voice_db(0),path_wav(""),pron(""),prefix(""),suffix(""),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),bwc(0)
{
  voice_db = other.voice_db;
  path_wav = other.path_wav;
  pron = other.pron;
  prefix = other.prefix;
  suffix = other.suffix;
  is_vcv = other.is_vcv;
  offs = other.offs;
  cons = other.cons;
  blnk = other.blnk;
  prec = other.prec;
  ovrl = other.ovrl;

  if (other.hasFrq()) {
    setFrq(other.getFrq());
  }

  if (other.hasBWC()) {
    setBWC(other.getBWC());
  }
}

Voice::~Voice()
{
  if (bwc != 0) {
    delete bwc;
    bwc = 0;
  }
}

Voice& Voice::operator=(const Voice& other)
{
  if (this != &other) {
    voice_db = other.voice_db;
    path_wav = other.path_wav;
    pron = other.pron;
    prefix = other.prefix;
    suffix = other.suffix;
    is_vcv = other.is_vcv;
    offs = other.offs;
    cons = other.cons;
    blnk = other.blnk;
    prec = other.prec;
    ovrl = other.ovrl;
    if (other.hasFrq()) {
      setFrq(other.getFrq());
    }
    if (other.hasBWC()) {
      setBWC(other.getBWC());
    }
  }
  return *this;
}

bool Voice::operator==(const Voice& other) const
{
  bool is_eq = true;

  is_eq &= (path_wav == other.path_wav);
  is_eq &= (pron == other.pron);
  is_eq &= (prefix == other.prefix);
  is_eq &= (suffix == other.suffix);
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

  if (hasBWC() && other.hasBWC()) {
    is_eq &= (getBWC() == other.getBWC());
  } else {
    is_eq &= (hasBWC() == other.hasBWC());
  }

  return is_eq;
}

bool Voice::operator!=(const Voice& other) const
{
  return !(*this == other);
}

void Voice::setWavPath(string path_wav)
{
  this->path_wav = path_wav;
}

bool Voice::hasFrq() const
{
  return frq > 0;
}

double Voice::getFrq() const
{
  if (frq > 0.0) {
    return frq;
  }

  ifstream ifs_frq(((path_wav.parent_path()/path_wav.stem()).string()+"_wav.frq").c_str(), ios::binary);
  if (ifs_frq.fail()) {
    return 260.0;
  }

  ifs_frq.seekg(sizeof(char)*12, ios_base::beg);
  ifs_frq.read((char*)&(this->frq), sizeof(double));
  return this->frq;
}

void Voice::setFrq(double frq)
{
  this->frq = frq;
}

bool Voice::hasBWC() const
{
  return (bwc!=0);
}

const BaseWavsContainer* Voice::getBWC() const
{
  if (bwc != 0) {
    return bwc;
  }

  this->bwc = new BaseWavsContainer;
  string alias = prefix+pron+suffix;
  boost::filesystem::path path_bwc(path_wav.parent_path().string()+"/"+boost::algorithm::replace_all_copy(alias, "*", "_")+".bwc");

  cout << "  loading voice \"" << alias << "\" from ";

  if (nak::cache && bwc::isBaseWavsContainerFile(path_bwc.string())) {
    cout << "cache" << endl;
    *bwc = bwc::get(path_bwc.string());
    return bwc;
  }

  // get wav data
  vector<short> wav_data;
  unsigned long wav_fs = 0;
  {
    cout << "wav data" << endl;
    WavParser wav_parser(path_wav.string());
    wav_parser.addTargetTrack(0);
    if (!wav_parser.parse()) {
      return 0;
    }
    wav_data = (*(wav_parser.getDataChunks().begin())).getDataVector();
    wav_fs = wav_parser.getFormat().dwSamplesPerSec;
  }

  // make input pitch mark
  vector<long> input_pitch_marks;
  {
    PitchMarker *marker = new PitchMarker();
    marker->setInputWav(wav_data, offs, ovrl, prec, blnk, wav_fs);
    if (nak::pron2vow.count(alias) > 0) {
      short win_size = wav_fs / getFrq() * 2;
      vector<short> aft_vowel_wav = voice_db->getVowel(nak::pron2vow[pron]+suffix);
      trimVector(&aft_vowel_wav, win_size);
      if (is_vcv && voice_db->isVowel(prefix+suffix)) {
        vector<short> fore_vowel_wav = voice_db->getVowel(prefix+suffix);
        trimVector(&fore_vowel_wav, win_size);
        marker->mark(fore_vowel_wav, aft_vowel_wav);
      } else {
        marker->mark(aft_vowel_wav);
      }
    } else {
      marker->mark(getFrq(), wav_fs);
    }
    input_pitch_marks = marker->getPitchMarks();
    delete marker;
  }

  // make base waves
  {
    BaseWavsMaker *maker = new BaseWavsMaker();
    maker->setPitchMarks(input_pitch_marks, offs+cons, offs+ovrl, wav_fs);
    maker->makeBaseWavs(wav_data, wav_fs/getFrq(), is_vcv);
    bwc->base_wavs = maker->getBaseWavs();
    bwc->format.wLobeSize = nak::base_wavs_lobe;
    bwc->format.dwRepeatStart = maker->getRepStartSub();
    bwc->format.wF0 = getFrq();
    delete maker;
  }

  // output bwc
  if (nak::cache) {
    bwc->format.setDefaultValues();
    bwc->format.chunkSize += BaseWavsFormat::wAdditionalSize + sizeof(short);
    bwc->format.wFormatTag = BaseWavsFormat::BaseWavsFormatTag;
    bwc->format.dwSamplesPerSec = wav_fs;
    bwc::set(path_bwc.string(), bwc);
  }

  return this->bwc;
}

void Voice::setBWC(const BaseWavsContainer *bwc)
{
  if (this->bwc != 0) {
    delete this->bwc;
    this->bwc = 0;
  }
  *(this->bwc) = *bwc;
}

template <class Vector>
Vector* Voice::trimVector(Vector* target_vector, long target_length) const
{
  if (target_vector->size() > target_length) {
    short space = target_vector->size() - target_length;
    target_vector->erase(target_vector->begin(), target_vector->begin()+space/2);
    target_vector->erase(target_vector->end()-(space-(space/2)));
  } else if (target_vector->size() < target_length) {
    short space = target_length - target_vector->size();
    target_vector->insert(target_vector->begin(), space/2, 0);
    target_vector->insert(target_vector->end(), space-(space/2), 0);
  }

  return target_vector;
}
