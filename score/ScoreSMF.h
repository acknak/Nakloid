#ifndef ScoreSMF_h
#define ScoreSMF_h

#include <list>
#include <string>
#include <boost/algorithm/string.hpp>
#include "Score.h"
#include "Note.h"
#include "../parser/SmfParser.h"
#include "../parser/SmfHandler.h"

// Reference Object
class ScoreSMF : public Score, public SmfHandler {
 public:
  ScoreSMF(std::string input_smf, short track, std::string path_lyric, std::string path_pitches, std::string path_song, std::string path_singer);
  virtual ~ScoreSMF();

  bool load(std::string input_smf, short track, std::string path_lyric);

  // extension method
  void smfInfo(short, short);
  void trackChange(short);
  void eventMidi(long, unsigned char, unsigned char*);
  void eventSysEx(long, long, unsigned char*);
  void eventMeta(long, unsigned char, long, unsigned char*);

 private:
  ScoreSMF(const ScoreSMF& other);
  ScoreSMF& operator=(const ScoreSMF& other);

  std::string singer;
  std::string path_song;
  unsigned short timebase;
  unsigned long tempo;
  std::vector<float> pitches;
  unsigned short track;
  bool is_parse;
  unsigned long time_parse;
  Note *note_parse;
  unsigned long id_parse;
};

#endif
