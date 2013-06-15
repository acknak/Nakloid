#ifndef Score_h
#define Score_h

#include <map>
#include <list>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>
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

  bool loadModifierMap(std::string path_modifier_map);
  std::pair<std::string, std::string> getModifier(short key);

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
  Note* getNextNote(Note *note);
  Note* getPrevNote(Note *note);

 private:
  Score(const Score& other);
  Score& operator=(const Score& other);
  std::vector<std::string> key2notenum;

 protected:
  std::vector<float> pitches;
  std::string path_song;
  std::string path_singer;
  bool is_tempered;
  std::map<short, std::pair<std::string, std::string> > key2modifier;
};

#endif
