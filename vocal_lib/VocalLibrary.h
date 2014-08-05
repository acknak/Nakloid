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
#include "../utilities/Tools.h"

class VocalLibrary {
 public:
  static struct Parameters {
    Parameters() {
      num_default_uwc_lobes = 3;
    }
    short num_default_uwc_lobes;
  } params;

  VocalLibrary():path_singer(L""){}
  explicit VocalLibrary(const boost::filesystem::path& path_singer):path_singer(path_singer){};
  virtual ~VocalLibrary();

  bool initVoiceMap();
  bool initVoiceMap(const boost::filesystem::path& path_oto_ini);

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
};

#endif
