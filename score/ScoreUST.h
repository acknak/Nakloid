#ifndef ScoreUST_h
#define ScoreUST_h

#include <list>
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

class ScoreUST : public Score {
 public:
  ScoreUST(const std::wstring& input_ust, const std::wstring& path_song, const std::wstring& path_singer);
  virtual ~ScoreUST();

  void load(const std::wstring& input_ust);

 protected:
  long id_parse;

 private:
  ScoreUST(const ScoreUST& other);
  ScoreUST& operator=(const ScoreUST& other);
};

#endif
