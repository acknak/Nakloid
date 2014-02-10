#ifndef ScoreSMF_h
#define ScoreSMF_h

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include "Note.h"
#include "Score.h"
#include "../parser/SmfHandler.h"
#include "../parser/SmfParser.h"

class ScoreSMF : public Score, public SmfHandler {
 public:
  ScoreSMF(const std::wstring &path_score, const VoiceDB *voice_db, const std::wstring &path_song, short track, const std::wstring& path_lyrics);
  virtual ~ScoreSMF();

  void load();

  // inherit from SmfParser 
  void smfInfo(short, short);
  void trackChange(short);
  void eventMidi(long, unsigned char, const unsigned char* const);
  void eventSysEx(long, long, const unsigned char* const);
  void eventMeta(long, unsigned char, long, const unsigned char* const);

 protected:
  std::wstring path_lyrics;
  std::vector<std::wstring> lyrics;
  unsigned short timebase;
  unsigned long tempo;
  unsigned short track;
  bool is_parse;
  unsigned long time_parse;
  Note *note_parse;
  unsigned long id_parse;

 private:
  ScoreSMF(const ScoreSMF& other);
  ScoreSMF& operator=(const ScoreSMF& other);
};

#endif
