#ifndef Score_h
#define Score_h

#include <list>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "Note.h"
#include "parser/SmfParser.h"
#include "parser/SmfHandler.h"

// Reference Object
class Score : public SmfHandler {
 public:
  Score();
  explicit Score(std::string input_ust);
  Score(std::string singer, std::string input_smf, short track, std::list<std::string> prons, std::string path_song);
  virtual ~Score();

  bool isScoreLoaded();
  void loadSmf(std::string input, unsigned short track, std::list<std::string> prons);
  void loadUst(std::string input);
  void reloadPitches();
  void debug(std::string output);

  // accessor
  std::string getSinger();
  void setSinger(std::string singer);
  std::string getSongPath();
  void setSongPath(std::string path_song);
  std::list<Note> getNotesList();
  std::vector<Note> getNotesVector();
  void setNotes(std::list<Note> notes);
  void setNotes(std::vector<Note> notes);
  std::vector<double> getPitches();
  void setPitches(std::vector<double> pitches);

  // extension method
  void smfInfo(short, short);
  void trackChange(short);
  void eventMidi(long, unsigned char, unsigned char*);
  void eventSysEx(long, long, unsigned char*);
  void eventMeta(long, unsigned char, long, unsigned char*);

 private:
  Score(const Score& other);
  Score& operator=(const Score& other);

  std::string singer;
  std::string path_song;
  unsigned short timebase;
  unsigned long tempo;
  std::list<Note> notes;
  std::vector<double> pitches;
  unsigned short track;
  bool is_parse;
  unsigned long time_parse;
  Note *note_parse;
};

#endif
