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
#include "../format/Wav.h"
#include "../vocal_lib/Voice.h"
#include "../vocal_lib/VocalLibrary.h"
class Note;

class Score {
 public:
  static struct Parameters {
    Parameters() {
      ms_margin = 0;
      smf_track = 1;
      pitch_frame_length = 5;
      auto_vowel_combining = false;
      vibrato = false;
      ms_vibrato_offset = 200;
      ms_vibrato_width = 200;
      pitch_vibrato = 30;
      overshoot = false;
      ms_overshoot = 50;
      pitch_overshoot = 30;
      preparation = false;
      ms_preparation = 50;
      pitch_preparation = 30;
      finefluctuation = false;
      finefluctuation_deviation = 0.5;
    }
    long ms_margin;
    short smf_track;
    short pitch_frame_length;
    bool auto_vowel_combining;
    bool vibrato;
    short ms_vibrato_offset, ms_vibrato_width;
    double pitch_vibrato;
    bool overshoot;
    short ms_overshoot;
    double pitch_overshoot;
    bool preparation;
    short ms_preparation;
    double pitch_preparation;
    bool finefluctuation;
    double finefluctuation_deviation;
    WavHeader wav_header;
  } params;

  Score(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib);
  Score(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib, const boost::filesystem::path& path_song);
  virtual ~Score();

  virtual void load(){};
  void saveScore(const boost::filesystem::path& path_score);
  bool loadModifierMap(const boost::filesystem::path& path_modifier_map);
  bool loadPitPitches(const boost::filesystem::path& path_pitches);
  bool loadLf0Pitches(const boost::filesystem::path& path_pitches);
  void savePitches(const boost::filesystem::path& path_pitches);

  // accessor
  const boost::filesystem::path& getScorePath() const;
  const boost::filesystem::path& getSongPath() const;
  void setSongPath(const boost::filesystem::path& paht_song);
  long getMargin() const;
  void setMargin(long margin);
  std::vector<Note>::const_iterator getNotesBegin() const;
  std::vector<Note>::const_iterator getNotesEnd() const;
  const Note* getNextNote(const Note *note) const;
  const Note* getPrevNote(const Note *note) const;
  const std::pair<std::wstring, std::wstring>& getModifier(short key) const;
  const std::vector<float>& getPitches() const;
  std::vector<long> getPitchMarks() const;

 protected:
  long ms_margin;
  boost::filesystem::path path_song;
  const VocalLibrary *vocal_lib;
  const static std::vector<std::wstring> key2notenum;

  void addNote(Note note);
  void deleteNote(std::vector<Note>::iterator it_notes);
  void clearNotes();
  void setPitches(const std::vector<float>& pitches);
  void reloadPitches();
  void clearPitches();

  inline long ms2pos(long ms, WavHeader header) const { return (long)(ms/1000.0*header.dwSamplesPerSec); }
  inline long pos2ms(long pos, WavHeader header) const { return (long)(pos/(double)header.dwSamplesPerSec * 1000); }
  inline long tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo) const { return (long)(((double)tick) / timebase*(tempo / 1000.0)); }
  inline double cent2rate(double cent) const { return pow(2, cent / 1200.0); }

 private:
  std::vector<float> pitches;
  std::vector<Note> notes;
  boost::filesystem::path path_score;
  std::map<short, std::pair<std::wstring, std::wstring> > key2modifier;

  void sanitizeNote(std::vector<Note>::iterator it_notes);

  Score(const Score& other);
  Score& operator=(const Score& other);
};

#endif
