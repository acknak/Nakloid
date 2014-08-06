#ifndef ScoreNAK_h
#define ScoreNAK_h

#include <cmath>
#include <list>
#include <string>
#include <tuple>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "Note.h"
#include "../parser/SmfParser.h"
#include "../parser/SmfHandler.h"

class ScoreNAK : public Score {
 public:
  ScoreNAK(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib, const boost::filesystem::path& path_song);
  virtual ~ScoreNAK();

  void load();

 private:
  ScoreNAK(const ScoreNAK& other);
  ScoreNAK& operator=(const ScoreNAK& other);

  Note *note_parse;
  long id_parse;
};

#endif
