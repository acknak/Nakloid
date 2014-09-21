#ifndef ScoreSMF_h
#define ScoreSMF_h

#include "Score.h"

#include "../parser/SmfHandler.h"

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>

class Note;

class ScoreSMF : public Score, public SmfHandler {
 public:
  ScoreSMF(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib, const boost::filesystem::path& path_song, const boost::filesystem::path& path_lyrics)
    :Score(path_score, vocal_lib, path_song),path_lyrics(path_lyrics),timebase(0),tempo(0),is_parse(false),time_parse(0),id_parse(0){}
  virtual ~ScoreSMF(){}

  void load();

 private:
  ScoreSMF(const ScoreSMF& other);
  ScoreSMF& operator=(const ScoreSMF& other);

  // inherit from SmfHandler
  void smfInfo(short, short);
  void trackChange(short);
  void eventMidi(long, unsigned char, const unsigned char* const);
  void eventSysEx(long, long, const unsigned char* const);
  void eventMeta(long, unsigned char, long, const unsigned char* const);

  boost::filesystem::path path_lyrics;
  std::vector<std::wstring> lyrics;
  unsigned short timebase;
  unsigned long tempo;
  bool is_parse;
  unsigned long time_parse;
  Note *note_parse;
  unsigned long id_parse;
};

#endif
