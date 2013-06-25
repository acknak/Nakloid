#ifndef ScoreNAK_h
#define ScoreNAK_h

#include <list>
#include <cmath>
#include <tuple>
#include <string>
#include <cstdlib>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Note.h"
#include "../Utilities.h"
#include "../parser/SmfParser.h"
#include "../parser/SmfHandler.h"

// Reference Object
class ScoreNAK : public Score {
 public:
  ScoreNAK(std::string input_nak, std::string path_song, std::string path_singer);
  virtual ~ScoreNAK();

  void load(std::string path_nak);

 private:
  ScoreNAK(const ScoreNAK& other);
  ScoreNAK& operator=(const ScoreNAK& other);

  Note *note_parse;
  long id_parse;
};

#endif
