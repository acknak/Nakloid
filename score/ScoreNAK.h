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
#include "../Utilities.h"

class ScoreNAK : public Score {
 public:
  ScoreNAK(const std::wstring& input_nak, const std::wstring& path_song, const std::wstring& path_singer);
  virtual ~ScoreNAK();

  void load(const std::wstring& path_nak);

 protected:
  Note *note_parse;
  long id_parse;

 private:
  ScoreNAK(const ScoreNAK& other);
  ScoreNAK& operator=(const ScoreNAK& other);
};

#endif
