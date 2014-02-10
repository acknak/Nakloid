#ifndef Score_h
#define Score_h

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/random.hpp>
#include "Note.h"
#include "../voiceDB/Voice.h"
class Note;

class Score {
 public:
  Score(const std::wstring& path_score, const VoiceDB *voice_db);
  Score(const std::wstring& path_score, const VoiceDB *voice_db, const std::wstring& path_song);
  virtual ~Score();

  virtual void load(){};
  void saveScore(const std::wstring& path_score);
  bool loadModifierMap(const std::wstring& path_modifier_map);
  bool loadPitches(const std::wstring& path_pitches, nak::PitchesMode pitches_mode);
  void savePitches(const std::wstring& path_pitches);

  // accessor
  const std::wstring& getScorePath() const;
  const std::wstring& getSongPath() const;
  void setSongPath(const std::wstring& paht_song);
  long getMargin() const;
  void setMargin(long margin);
  std::vector<Note>::const_iterator getNotesBegin() const;
  std::vector<Note>::const_iterator getNotesEnd() const;
  const Note* getNextNote(const Note *note) const;
  const Note* getPrevNote(const Note *note) const;
  const std::pair<std::wstring, std::wstring>& getModifier(short key) const;
  const std::vector<float>& getPitches() const;
  std::vector<long> getPitchMarks(const WavFormat& format) const;

 protected:
  long ms_margin;
  std::wstring path_song;
  const VoiceDB *voice_db;
  const static std::vector<std::wstring> key2notenum;

  void addNote(Note note);
  void deleteNote(std::vector<Note>::iterator it_notes);
  void clearNotes();
  void setPitches(const std::vector<float>& pitches);
  void reloadPitches();
  void clearPitches();

 private:
  std::vector<float> pitches;
  std::vector<Note> notes;
  std::wstring path_score;
  std::map<short, std::pair<std::wstring, std::wstring> > key2modifier;

  void sanitizeNote(std::vector<Note>::iterator it_notes);

  Score(const Score& other);
  Score& operator=(const Score& other);
};

#endif
