#ifndef ScoreUST_h
#define ScoreUST_h

#include <list>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "Score.h"
#include "Note.h"
#include "../parser/SmfParser.h"
#include "../parser/SmfHandler.h"

// Reference Object
class ScoreUST : public Score, public SmfHandler {
 public:
  ScoreUST(std::string input_ust, std::string path_pitches, std::string path_song, std::string path_singer);
  virtual ~ScoreUST();

  void load(std::string input_ust);

 private:
  ScoreUST(const ScoreUST& other);
  ScoreUST& operator=(const ScoreUST& other);

  Note *note_parse;
  unsigned long id_parse;
};

#endif
