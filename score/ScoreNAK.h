#ifndef ScoreNAK_h
#define ScoreNAK_h

#include <cmath>
#include <list>
#include <string>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/stringbuffer.h>
#include "Note.h"
#include "Score.h"
#include "../parser/SmfParser.h"
#include "../parser/SmfHandler.h"
#include "Score.h"

#include <boost/filesystem/path.hpp>

class Note;
class VocalLibrary;

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
