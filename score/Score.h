#ifndef Score_h
#define Score_h

#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Note.h"
class Note;

// Reference Object
class Score {
 public:
  Score(std::string input, std::string path_song, std::string path_singer);
  virtual ~Score();

  virtual bool load();
  bool isScoreLoaded();
  void reloadPitches();

  void saveScore(std::string path_nak);
  bool loadPitchesFromPit(std::string path_input_pitches);
  bool loadPitchesFromLf0(std::string path_input_pitches);
  void savePitches(std::string path_output_pitches);

  // Note mediator
  short getNoteLack(Note *note);
  long getNoteNextDist(Note *note);
  long getNotePrevDist(Note *note);

  // member
  std::list<Note> notes;

  // accessor
  std::vector<float> getPitches();
  void setPitches(std::vector<float> pitches);
  std::string getSongPath();
  void setSongPath(std::string path_song);
  std::string getSingerPath();
  void setSingerPath(std::string paht_singer);
  bool isTempered();

 private:
  Score(const Score& other);
  Score& operator=(const Score& other);

 protected:
  std::vector<float> pitches;
  std::string path_song;
  std::string path_singer;
  bool is_tempered;
};

#endif
