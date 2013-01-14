#ifndef ScoreNML_h
#define ScoreNML_h

#include <list>
#include <cmath>
#include <string>
#include "Note.h"
#include "../parser/SmfParser.h"
#include "../parser/SmfHandler.h"

// Reference Object
class ScoreNML : public Score, public SmfHandler {
 public:
  ScoreNML(std::string input_nml, std::string path_pitches, std::string path_song, std::string path_singer);
  virtual ~ScoreNML();

  void load(std::string input_nml);

 private:
  ScoreNML(const ScoreNML& other);
  ScoreNML& operator=(const ScoreNML& other);

  Note *note_parse;
  unsigned long id_parse;
};

#endif
