#ifndef Voice_h
#define Voice_h

#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "VoiceDB.h"
#include "CacheFileIO.h"
#include "../Utilities.h"
#include "../PSOLA/UnitWaveform.h"
class VoiceDB;

// Value Object
class Voice {
 public:
  Voice();
  Voice(VoiceDB* voice_db);
  Voice(const Voice& other);
  ~Voice();
  
  Voice& operator=(const Voice& other);
  bool operator==(const Voice& other) const;
  bool operator!=(const Voice& other) const;

  VoiceDB* voice_db;
  boost::filesystem::path path_wav;
  std::string pron;
  std::string prefix;
  std::string suffix;
  bool is_vcv;
  short offs; // offset(left blank)
  short cons; // consonant part(unaltered range)
  short blnk; // blank(right blank)
  short prec; // preceding utterance
  short ovrl; // overlap range

  void setWavPath(std::string path_wav);
  bool hasFrq() const;
  double getFrq() const;
  void setFrq(double frq);
  bool hasUwc() const;
  const uw::UnitWaveformContainer* getUwc() const;
  void setUwc(const uw::UnitWaveformContainer *uwc);

 private:
  template <class Vector>
  Vector* trimVector(Vector* target_vector, long target_length) const;
  mutable double frq;
  mutable uw::UnitWaveformContainer* uwc;
};

#endif
