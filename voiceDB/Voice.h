#ifndef Voice_h
#define Voice_h

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include "CacheFileIO.h"
#include "VoiceDB.h"
#include "../Utilities.h"
#include "../PSOLA/UnitWaveform.h"
class VoiceDB;

class Voice {
 public:
  Voice();
  Voice(VoiceDB* const voice_db);
  Voice(const Voice& other);
  virtual ~Voice();
  
  Voice& operator=(const Voice& other);
  bool operator==(const Voice& other) const;
  bool operator!=(const Voice& other) const;

  VoiceDB* voice_db;
  boost::filesystem::path path_wav;
  nak::VoiceAlias alias;
  bool is_vcv;
  short offs; // offset(left blank)
  short cons; // consonant part(unaltered range)
  short blnk; // blank(right blank)
  short prec; // preceding utterance
  short ovrl; // overlap range

  // accessor
  const nak::VoiceAlias& getAlias() const;
  std::wstring getAliasString() const;
  const std::wstring& getPrefix() const;
  const std::wstring& getPron() const;
  const std::wstring& getSuffix() const;
  void setAlias(const std::wstring &alias);
  void setWavPath(const std::wstring &path_wav);
  bool hasFrq() const;
  float getFrq() const;
  void setFrq(double frq);
  bool hasUwc() const;
  const uw::UnitWaveformContainer* getUwc() const;
  void setUwc(const uw::UnitWaveformContainer &uwc);

 protected:
  template <class Vector>
  Vector* trimVector(Vector* target_vector, long target_length) const;
  mutable float frq;
  mutable uw::UnitWaveformContainer* uwc;
};

#endif
