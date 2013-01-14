#ifndef Score_h
#define Score_h

#include <list>
#include <string>
#include <boost/utility.hpp>
#include "Note.h"
class Note;

// Reference Object
class Score {
 public:
  Score(std::string input, std::string path_pitches, std::string path_song, std::string path_singer);
  virtual ~Score();

  virtual bool load();
  bool isScoreLoaded();
  void reloadPitches();

  // Note mediator
  void noteParamChanged(Note *note);

  // member
  std::list<Note> notes;

  // accessor
  std::vector<float> getPitches();
  void setPitches(std::vector<float> pitches);
  std::string getSongPath();
  void setSongPath(std::string path_song);
  std::string getSingerPath();
  void setSingerPath(std::string paht_singer);

 private:
  Score(const Score& other);
  Score& operator=(const Score& other);

 protected:
  std::vector<float> pitches;
  std::string path_song;
  std::string path_singer;
};

#endif
