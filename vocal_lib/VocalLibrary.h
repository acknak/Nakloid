#ifndef VocalLibrary_h
#define VocalLibrary_h

#include <cctype>
#include <string>
#include <tuple>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "Voice.h"
#include "VoiceUWC.h"
#include "VoiceWAV.h"
#include "../format/PronunciationAlias.h"
#include "../format/UnitWaveformContainer.h"

class VocalLibrary {
 public:
  static struct Parameters {
    Parameters() {
      num_default_uwc_lobes = 1;
      use_uwc_cache = true;
    }
    short num_default_uwc_lobes;
    bool use_uwc_cache;
  } params;

  VocalLibrary():path_singer(L""){}
  explicit VocalLibrary(const boost::filesystem::path& path_singer):path_singer(path_singer){};
  virtual ~VocalLibrary();

  bool initVoiceMap(bool make_cache_file_mode=false);
  bool initVoiceMap(const boost::filesystem::path& path_oto_ini, bool make_cache_file_mode=false);
  bool makeFileCache(const PronunciationAlias& pron_alias) const;

  // accessor
  const Voice* getVoice(const std::wstring& alias) const;
  bool isAlias(const std::wstring& alias) const;
  void setSingerPath(const boost::filesystem::path& path_singer);
  const boost::filesystem::path& getSingerPath() const;

 private:
  boost::filesystem::path path_singer;
  std::map<std::wstring, Voice*> voice_map;
  unsigned char num_default_uwc_lobes;

 private:
  VocalLibrary(const VocalLibrary& other);
  VocalLibrary& operator=(const VocalLibrary& other);

  void parseVoiceData(Voice* voice, const std::vector<std::wstring>& data) const;
};

#endif
