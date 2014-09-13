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

bool VocalLibrary::initVoiceMap()
{
  namespace fs = boost::filesystem;
  const fs::path path(path_singer);

  if (fs::is_directory(path)) {
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(path), fs::recursive_directory_iterator())) {
      if (p.leaf().wstring() == L"oto.ini") {
        wcout << L"loading " << p << endl;
        initVoiceMap(p);
      }
    }
    if (voice_map.size() > 0)
      return true;
  }

  wcerr << L"[VocalLibrary::initVoiceMap] path_singer: " << path_singer << L" is invalid" << endl;
  return false;
}

bool VocalLibrary::initVoiceMap(const boost::filesystem::path& path_oto_ini)
{
  boost::filesystem::wifstream ifs(path_oto_ini);
  boost::filesystem::path path_ini(path_oto_ini);
  wstring buf, wav_ext=L".wav";
  while (ifs && getline(ifs, buf)) {
    // read oto.ini
    vector<wstring> v1, v2;
    boost::algorithm::split(v1, buf, boost::is_any_of("="));
    boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
    Voice *tmp_voice;
    {
      boost::filesystem::path path_wav = path_ini.parent_path()/v1[0];
      wstring str_pron_alias = (v2[0].empty())?path_wav.stem().wstring():v2[0];
      boost::filesystem::path path_uwc = path_wav.parent_path()/boost::algorithm::replace_all_copy((str_pron_alias+L".uwc"), L"*", L"_");
      if (params.uwc_cache && UnitWaveformContainer::isUwcFormatFile(path_uwc)) {
        tmp_voice = new VoiceUWC(str_pron_alias, path_uwc);
      } else {
        tmp_voice = new VoiceWAV(str_pron_alias, path_wav);
      }
    }
    for (size_t i=0; i<v2.size(); i++) {
      boost::algorithm::trim(v2[i]);
    }
    short tmp;
    tmp_voice->setOffs((((tmp=boost::lexical_cast<double>(v2[1]))>0))?tmp:0);
    tmp_voice->setCons((((tmp=boost::lexical_cast<double>(v2[2]))>0))?tmp:0);
    tmp_voice->setBlnk(boost::lexical_cast<double>(v2[3]));
    tmp_voice->setPrec(boost::lexical_cast<double>(v2[4]));
    tmp_voice->setOvrl(boost::lexical_cast<double>(v2[5]));

    // sanitize
    if (tmp_voice->getOvrl() > tmp_voice->getPrec()) {
      tmp_voice->setPrec(tmp_voice->getOvrl());
    }
    if (tmp_voice->getPrec() > tmp_voice->getCons()) {
      tmp_voice->setCons(tmp_voice->getPrec());
    }
    if (tmp_voice->getBlnk()<0 && tmp_voice->getCons()>-tmp_voice->getBlnk()) {
      tmp_voice->setBlnk(-tmp_voice->getCons());
    }
    if (tmp_voice->getOffs() < 0) {
      short tmp = -tmp_voice->getOffs();
      tmp_voice->setOffs(0);
      tmp_voice->setOvrl(tmp_voice->getOvrl()+tmp);
      tmp_voice->setCons(tmp_voice->getCons()+tmp);
      tmp_voice->setPrec(tmp_voice->getPrec()+tmp);
      if (tmp_voice->getBlnk() < 0) {
        tmp_voice->setBlnk(tmp_voice->getBlnk()-tmp);
      }
    }
    voice_map[tmp_voice->getPronAliasString()] = tmp_voice;
  }
  return true;
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
