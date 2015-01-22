#ifndef ScoreUST_h
#define ScoreUST_h

#include "Score.h"

#include <boost/filesystem/path.hpp>

class VocalLibrary;

class ScoreUST : public Score {
 public:
  ScoreUST(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib, const boost::filesystem::path& path_song);
  virtual ~ScoreUST();

  void load();

 protected:
  long id_parse;

 private:
  ScoreUST(const ScoreUST& other);
  ScoreUST& operator=(const ScoreUST& other);
};

#endif
