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
      pitch_default = 260;
      uwc_cache = true;
      num_default_uwc_lobes = 3;
    }
    long pitch_default;
    bool uwc_cache;
    short num_default_uwc_lobes;
  } params;

  Voice(const std::wstring& str_pron_alias, boost::filesystem::path path)
    :path(path),pron_alias(PronunciationAlias(str_pron_alias)),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),uwc(0){}
  Voice(const PronunciationAlias& pron_alias, boost::filesystem::path path)
    :path(path),pron_alias(pron_alias),offs(0),cons(0),blnk(0),prec(0),ovrl(0),frq(0.0),uwc(0){}
  Voice(const Voice& other);
  virtual ~Voice();
  
  Voice& operator=(const Voice& other);
  bool operator==(const Voice& other) const;
  bool operator!=(const Voice& other) const;

  // accessor
  virtual const PronunciationAlias& getPronAlias() const;
  virtual std::wstring getPronAliasString() const;
  virtual const std::wstring& getPrefix() const;
  virtual const std::wstring& getPron() const;
  virtual const std::wstring& getSuffix() const;
  virtual const boost::filesystem::path& getPath() const;
  virtual bool hasFrq() const;
  virtual float getFrq() const;
  virtual void setFrq(double frq);
  virtual const UnitWaveformContainer* getUnitWaveformContainer() const{return 0;}
  virtual bool hasUnitWaveformContainer() const;
  virtual void setUnitWaveformContainer(const UnitWaveformContainer* uwc);
  virtual bool isVCV() const;

  virtual short getOffs() const;
  virtual void setOffs(short offs);
  virtual short getCons() const;
  virtual void setCons(short cons);
  virtual short getBlnk() const;
  virtual void setBlnk(short blnk);
  virtual short getPrec() const;
  virtual void setPrec(short prec);
  virtual short getOvrl() const;
  virtual void setOvrl(short ovrl);

 protected:
  inline void trimVector(std::vector<double>* target_vector, long target_length) const;
  inline double getRMS(const std::vector<double>::const_iterator from, const std::vector<double>::const_iterator to) const;

  boost::filesystem::path path;
  PronunciationAlias pron_alias;
  mutable float frq;
  mutable UnitWaveformContainer *uwc;

  short offs; // offset(left blank)
  short cons; // consonant part(unaltered range)
  short blnk; // blank(right blank)
  short prec; // preceding utterance
  short ovrl; // overlap range
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
