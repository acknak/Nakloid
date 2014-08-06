#ifndef Voice_h
#define Voice_h

#include <string>
#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "../format/PronunciationAlias.h"
#include "../format/UnitWaveformContainer.h"

class Voice {
 public:
  static struct Parameters {
    Parameters() {
      uwc_cache = true;
      num_default_uwc_lobes = 3;
    }
    bool uwc_cache;
    short num_default_uwc_lobes;
  } params;

  static std::map< std::wstring, std::vector<double> > vowel_wav_map;

  Voice()
    :path(L""),pron_alias(),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),uwc(0){}
  explicit Voice(boost::filesystem::path path)
    :path(path),pron_alias(),is_vcv(false),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),uwc(0){}
  Voice(const Voice& other);
  virtual ~Voice();
  
  Voice& operator=(const Voice& other);
  bool operator==(const Voice& other) const;
  bool operator!=(const Voice& other) const;

  boost::filesystem::path path;
  PronunciationAlias pron_alias;
  bool is_vcv;
  short offs; // offset(left blank)
  short cons; // consonant part(unaltered range)
  short blnk; // blank(right blank)
  short prec; // preceding utterance
  short ovrl; // overlap range

  // accessor
  const PronunciationAlias& getPronAlias() const;
  std::wstring getPronAliasString() const;
  const std::wstring& getPrefix() const;
  const std::wstring& getPron() const;
  const std::wstring& getSuffix() const;
  void setPronAlias(const std::wstring& pron_alias);
  void setPronAlias(const PronunciationAlias& pron_alias);
  void setPath(const boost::filesystem::path& path);
  bool hasFrq() const;
  float getFrq() const;
  void setFrq(double frq);
  virtual const UnitWaveformContainer* getUnitWaveformContainer() const{return 0;}
  bool hasUnitWaveformContainer() const;
  void setUnitWaveformContainer(const UnitWaveformContainer* uwc);

 protected:
  inline void trimVector(std::vector<double>* target_vector, long target_length) const;
  inline double getRMS(const std::vector<double>::const_iterator from, const std::vector<double>::const_iterator to) const;

  mutable float frq;
  mutable UnitWaveformContainer *uwc;
};

inline void Voice::trimVector(std::vector<double>* target_vector, long target_length) const
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
}

inline double Voice::getRMS(const std::vector<double>::const_iterator from, const std::vector<double>::const_iterator to) const
{
  double rms = 0.0;
  for (std::vector<double>::const_iterator it = from; it != to; ++it) {
    rms += pow((double)*it, 2) / (to - from);
  }
  return sqrt(rms);
}

#endif
