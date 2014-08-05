#ifndef ScoreUST_h
#define ScoreUST_h

#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include "Note.h"
#include "Score.h"
#include "../parser/SmfHandler.h"
#include "../parser/SmfParser.h"
#include "../utilities/Tools.h"

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
