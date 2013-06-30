#ifndef ScoreSMF_h
#define ScoreSMF_h

#include <list>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/fstream.hpp>
#include "Note.h"
#include "Score.h"
#include "../parser/SmfHandler.h"
#include "../parser/SmfParser.h"

class ScoreSMF : public Score, public SmfHandler {
 public:
  ScoreSMF(const std::wstring& input_smf, short track, const std::wstring& path_lyrics, const std::wstring& path_song, const std::wstring& path_singer);
  virtual ~ScoreSMF();

  bool load(const std::wstring& input_smf, short track, const std::wstring& path_lyrics);

  // inherit from SmfParser 
  void smfInfo(short, short);
  void trackChange(short);
  void eventMidi(long, unsigned char, const unsigned char* const);
  void eventSysEx(long, long, const unsigned char* const);
  void eventMeta(long, unsigned char, long, const unsigned char* const);

 protected:
  std::wstring singer;
  std::wstring path_song;
  unsigned short timebase;
  unsigned long tempo;
  std::vector<float> pitches;
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
