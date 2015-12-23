#include "VocalLibrary.h"

using namespace std;

struct VocalLibrary::Parameters VocalLibrary::params;

VocalLibrary::~VocalLibrary()
{
  for (map<wstring, Voice*>::iterator it=voice_map.begin(); it!=voice_map.end(); ++it) {
    delete it->second;
    it->second = 0;
  }
}

bool VocalLibrary::initVoiceMap(bool make_cache_file_mode)
{
  namespace fs = boost::filesystem;
  const fs::path path(path_singer);
  if (fs::is_directory(path)) {
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(path), fs::recursive_directory_iterator())) {
      if (p.leaf().wstring() == L"oto.ini") {
        wcout << L"loading " << p << endl;
        initVoiceMap(p, make_cache_file_mode);
      }
    }
    if (make_cache_file_mode || voice_map.size() > 0) {
      return true;
    }
  }
  wcerr << L"[VocalLibrary::initVoiceMap] path_singer: " << path_singer << L" is invalid" << endl;
  return false;
}

bool VocalLibrary::initVoiceMap(const boost::filesystem::path& path_oto_ini, bool make_cache_file)
{
  boost::filesystem::wifstream ifs(path_oto_ini);
  boost::filesystem::path path_ini(path_oto_ini);

  wstring buf, wav_ext=L".wav";
  while (ifs && getline(ifs, buf)) {
    // read oto.ini
    vector<wstring> v1, v2;
    boost::algorithm::split(v1, buf, boost::is_any_of("="));
    boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
    boost::filesystem::path path_wav = path_ini.parent_path()/v1[0];
    PronunciationAlias pron_alias((v2[0].empty())?path_wav.stem().wstring():v2[0]);
    boost::filesystem::path path_uwc = path_wav.parent_path()/boost::algorithm::replace_all_copy((pron_alias.getAliasString()+L".uwc"), L"*", L"_");
    for (size_t i=0; i<v2.size(); i++) {
      boost::algorithm::trim(v2[i]);
    }
    if (v2.size()<6 || v2[1].empty() || v2[2].empty() || v2[3].empty() || v2[4].empty() || v2[5].empty()) {
      wcerr << L"[VocalLibrary::makeFileCache] invalid parameter found at \"" << pron_alias.getAliasString() << L"\" of \"" << path_singer << L"\"" << endl;
      return false;
    }
    if (make_cache_file) {
      VoiceWAV *tmp_voice = new VoiceWAV(pron_alias, path_wav);
      parseVoiceData(tmp_voice, v2);
      tmp_voice->makeUnitWaveformContainerCache(false);
      delete tmp_voice;
    } else {
      Voice *tmp_voice;
      if (params.use_uwc_cache && UnitWaveformContainer::isUwcFormatFile(path_uwc)) {
        tmp_voice = new VoiceUWC(pron_alias, path_uwc);
      } else {
        tmp_voice = new VoiceWAV(pron_alias, path_wav);
      }
      parseVoiceData(tmp_voice, v2);
      voice_map[tmp_voice->getPronAliasString()] = tmp_voice;
    }
  }
  return true;
}

bool VocalLibrary::makeFileCache(const PronunciationAlias& pron_alias) const
{
  wstring str_pron_alias = pron_alias.getAliasString();
  namespace fs = boost::filesystem;
  const fs::path path(path_singer);
  if (fs::is_directory(path)) {
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(path), fs::recursive_directory_iterator())) {
      if (p.leaf().wstring() == L"oto.ini") {
        wcout << L"loading " << p << endl;
        boost::filesystem::wifstream ifs(p);
        boost::filesystem::path path_ini(p);
        wstring buf, wav_ext=L".wav";
        while (ifs && getline(ifs, buf)) {
          // read oto.ini
          vector<wstring> v1, v2;
          boost::algorithm::split(v1, buf, boost::is_any_of("="));
          boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
          boost::filesystem::path path_wav = path_ini.parent_path()/v1[0];
          if (v2.size()<6 || v2[1].empty() || v2[2].empty() || v2[3].empty() || v2[4].empty() || v2[5].empty()) {
            wcerr << L"[VocalLibrary::makeFileCache] invalid parameter found at \"" << str_pron_alias << L"\" of \"" << path_singer << L"\"" << endl;
            return false;
          }
          if (str_pron_alias == ((v2[0].empty())?path_wav.stem().wstring():v2[0])) {
            VoiceWAV *tmp_voice = new VoiceWAV(pron_alias, path_wav);
            parseVoiceData(tmp_voice, v2);
            tmp_voice->makeUnitWaveformContainerCache(false);
            delete tmp_voice;
            return true;
          }
        }
      }
    }
  }
  wcerr << L"[VocalLibrary::makeFileCache] can't find pron_alias \"" << str_pron_alias << L"\" at \"" << path_singer << L"\"" << endl;
  return false;
}

/*
 * accessor
 */
const Voice* VocalLibrary::getVoice(const wstring& alias) const
{
  if (!isAlias(alias)) {
    PronunciationAlias pron_alias(alias);
    if (pron_alias.prefix == L"- ") {
      pron_alias.prefix = L"";
      if (isAlias(pron_alias.getAliasString())) {
        wcout << L"[Voice::getVoice] replace pron_alias " << alias << L"with " << pron_alias.getAliasString() << endl;
        return voice_map.at(pron_alias.getAliasString());
      }
    } else if (pron_alias.prefix == L"") {
      pron_alias.prefix = L"- ";
      if (isAlias(pron_alias.getAliasString())) {
        wcout << L"[Voice::getVoice] replace pron_alias " << alias << L"with " << pron_alias.getAliasString() << endl;
        return voice_map.at(pron_alias.getAliasString());
      }
    }
    return 0;
  }

  return voice_map.at(alias);
}

bool VocalLibrary::isAlias(const wstring& alias) const
{
  return !(voice_map.empty() || voice_map.count(alias)==0);
}

void VocalLibrary::setSingerPath(const boost::filesystem::path& path_singer)
{
  this->path_singer = path_singer;
}

const boost::filesystem::path& VocalLibrary::getSingerPath() const
{
  return this->path_singer;
}

void VocalLibrary::parseVoiceData(Voice* voice, const vector<wstring>& data) const
{
  short tmp;
  voice->setOffs((((tmp=boost::lexical_cast<double>(data[1]))>0))?tmp:0);
  voice->setCons((((tmp=boost::lexical_cast<double>(data[2]))>0))?tmp:0);
  voice->setBlnk(boost::lexical_cast<double>(data[3]));
  voice->setPreu(boost::lexical_cast<double>(data[4]));
  voice->setOvrl(boost::lexical_cast<double>(data[5]));

  // sanitize
  if (voice->getOvrl() > voice->getPreu()) {
    voice->setPreu(voice->getOvrl());
  }
  if (voice->getPreu() > voice->getCons()) {
    voice->setCons(voice->getPreu());
  }
  if (voice->getBlnk()<0 && voice->getCons()>-voice->getBlnk()) {
    voice->setBlnk(-voice->getCons());
  }
  if (voice->getOffs() < 0) {
    short tmp = -voice->getOffs();
    voice->setOffs(0);
    voice->setOvrl(voice->getOvrl()+tmp);
    voice->setCons(voice->getCons()+tmp);
    voice->setPreu(voice->getPreu()+tmp);
    if (voice->getBlnk() < 0) {
      voice->setBlnk(voice->getBlnk()-tmp);
    }
  }
}