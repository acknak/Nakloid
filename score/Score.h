#ifndef Score_h
#define Score_h

#include <algorithm>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/utility.hpp>
#include "Note.h"
class Note;

class Score {
 public:
  Score(const std::wstring& input, const std::wstring& path_song, const std::wstring& path_singer);
  virtual ~Score();

  virtual bool load();
  bool isScoreLoaded() const;
  void reloadPitches();

  void saveScore(const std::wstring& path_nak);
  bool loadPitchesFromPit(const std::wstring& path_input_pitches);
  bool loadPitchesFromLf0(const std::wstring& path_input_pitches);
  void savePitches(const std::wstring& path_output_pitches);

  bool loadModifierMap(const std::wstring& path_modifier_map);
  const std::pair<std::wstring, std::wstring>& getModifier(short key) const;

  // accessor
  const std::vector<float>& getPitches() const;
  void setPitches(const std::vector<float>& pitches);
  const std::wstring& getSongPath() const;
  void setSongPath(const std::wstring& path_song);
  std::wstring getSingerPath() const;
  void setSingerPath(const std::wstring& paht_singer);
  bool isTempered() const;
  Note* getNextNote(Note *note);
  Note* getPrevNote(Note *note);

  std::list<Note> notes;

 protected:
  std::vector<float> pitches;
  std::wstring path_song;
  std::wstring path_singer;
  bool is_tempered;
  std::map<short, std::pair<std::wstring, std::wstring> > key2modifier;
  static std::vector<std::wstring> key2notenum;

 private:
  Score(const Score& other);
  Score& operator=(const Score& other);
};

#endif
