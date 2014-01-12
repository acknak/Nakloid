#include "VoiceDB.h"

using namespace std;

VoiceDB::VoiceDB():path_singer(L"") {}

VoiceDB::VoiceDB(const wstring& path_singer)
{
  setSingerPath(path_singer);
}

VoiceDB::~VoiceDB() {}

bool VoiceDB::initVoiceMap()
{
  namespace fs = boost::filesystem;
  const fs::path path(path_singer);

  if (fs::is_directory(path)) {
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(path), fs::recursive_directory_iterator())) {
      if (p.leaf().wstring() == L"oto.ini") {
        wcout << L"loading " << p.wstring() << endl;
        initVoiceMap(p.wstring());
      }
    }
    if (voice_map.size() > 0)
      return true;
  }

  cerr << "[VoiceDB::initVoiceMap] path_singer is invalid" << endl;
  return false;
}

bool VoiceDB::initVoiceMap(const wstring& path_oto_ini)
{
  boost::filesystem::wifstream ifs(path_oto_ini);
  boost::filesystem::path path_ini(path_oto_ini);
  wstring buf, wav_ext=L".wav";
  while (ifs && getline(ifs, buf)) {
    // read oto.ini
    Voice tmp_voice(this);
    vector<wstring> v1, v2;
    boost::algorithm::split(v1, buf, boost::is_any_of("="));
    boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
    short tmp;
    tmp_voice.setWavPath((path_ini.parent_path()/v1[0]).wstring());
    tmp_voice.offs = (((tmp=boost::lexical_cast<double>(v2[1]))>0))?tmp:0;
    tmp_voice.cons = (((tmp=boost::lexical_cast<double>(v2[2]))>0))?tmp:0;
    tmp_voice.blnk = boost::lexical_cast<double>(v2[3]);
    tmp_voice.prec = boost::lexical_cast<double>(v2[4]);
    tmp_voice.ovrl = boost::lexical_cast<double>(v2[5]);
    tmp_voice.is_vcv = false;
    // sanitize
    if (tmp_voice.ovrl > tmp_voice.prec) {
      tmp_voice.prec = tmp_voice.ovrl;
    }
    if (tmp_voice.prec > tmp_voice.cons) {
      tmp_voice.cons = tmp_voice.prec;
    }
    if (tmp_voice.blnk<0 && tmp_voice.cons > -tmp_voice.blnk) {
      tmp_voice.blnk = -tmp_voice.cons;
    }
    if (tmp_voice.offs < 0) {
      short tmp = -tmp_voice.offs;
      tmp_voice.offs = 0;
      tmp_voice.ovrl += tmp;
      tmp_voice.cons += tmp;
      tmp_voice.prec += tmp;
      if (tmp_voice.blnk < 0) {
        tmp_voice.blnk -= tmp;
      }
    }
    // get Voice pron
    tmp_voice.setAlias((v2[0]==L"")?tmp_voice.path_wav.stem().wstring():v2[0]);
    tmp_voice.is_vcv = (tmp_voice.alias.prefix!=L"- " && tmp_voice.alias.prefix!=L"* " && !tmp_voice.alias.prefix.empty());
    // set vowel_map
    if (!tmp_voice.is_vcv) {
      map<wstring, wstring>::const_iterator it = nak::getVow2PronIt(tmp_voice.getPron());
      if (it!=nak::vow2pron.end() && (tmp_voice.alias.prefix==L"- "||tmp_voice.alias.prefix.empty())) {
        WavParser wav_parser(tmp_voice.path_wav.wstring());
        wav_parser.addTargetTrack(0);
        if (wav_parser.parse()) {
          vector<double> tmp_wav = (*(wav_parser.getDataChunks().begin())).getData();
          vector<double>::iterator it_tmp_wav_cons = tmp_wav.begin()+((tmp_voice.offs+tmp_voice.cons)/1000.0*wav_parser.getFormat().dwSamplesPerSec);
          vector<double>::iterator it_tmp_wav_min = it_tmp_wav_cons;
          short win_size = wav_parser.getFormat().dwSamplesPerSec / tmp_voice.getFrq();
          double tmp_min_rms = -1.0;
          for (size_t i=0; i<win_size*2; i++) {
            vector<double> tmp_wav(it_tmp_wav_cons+i-win_size, it_tmp_wav_cons+i+win_size);
            double tmp_rms = nak::getRMS(tmp_wav);
            if (tmp_rms<tmp_min_rms || tmp_min_rms<0) {
              tmp_min_rms = tmp_rms;
              it_tmp_wav_min = it_tmp_wav_cons+i;
            }
          }
          vowel_map[nak::pron2vow[it->second]+tmp_voice.alias.suffix].assign(it_tmp_wav_min-win_size, it_tmp_wav_min+win_size);
        }
      }
    }
    voice_map[tmp_voice.getAliasString()] = tmp_voice;
  }
  return true;
}

/*
 * accessor
 */
const Voice* VoiceDB::getVoice(const wstring& alias) const
{
  if (!isAlias(alias))
    return 0;

  return &(voice_map.at(alias));
}

bool VoiceDB::isAlias(const wstring& alias) const
{
  return !(voice_map.empty() || voice_map.count(alias)==0);
}

bool VoiceDB::isVowel(const wstring& subject) const
{
  return vowel_map.count(subject)>0;
}

const vector<double>& VoiceDB::getVowel(const wstring& subject) const
{
  return vowel_map.at(subject);
}

void VoiceDB::setSingerPath(const wstring& path_singer)
{
  this->path_singer = path_singer;
}

const wstring& VoiceDB::getSingerPath() const
{
  return this->path_singer;
}
