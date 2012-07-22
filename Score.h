#ifndef Score_h
#define Score_h

#include <list>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "Note.h"
#include "parser/SmfParser.h"
#include "parser/SmfHandler.h"

// Reference Object
class Score : public SmfHandler {
 public:
  Score();
  Score(std::string input, short track);
  virtual ~Score();

  void load(std::string input);
  bool isScoreLoaded();
  void debug(std::string output);

  // accessor
  void setTrack(short track);
  std::list<Note> getNotesList();
  std::vector<Note> getNotesVector();
  void setNote(std::list<Note> notes);

  // extension method
  void smfInfo(short, short);
  void trackChange(short);
  void eventMidi(long, unsigned char, unsigned char*);
  void eventSysEx(long, long, unsigned char*);
  void eventMeta(long, unsigned char, long, unsigned char*);

 private:
  Score(const Score& other);
  Score& operator=(const Score& other);

  double tickToMSec(unsigned long tick);

  unsigned short timebase;
  unsigned long tempo;
  std::list<Note> notes;
  unsigned short track;
  bool is_parse;
  unsigned long time_parse;
  Note *note_parse;
};

#endif
