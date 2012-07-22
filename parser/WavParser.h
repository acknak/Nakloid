#ifndef WavParser_h
#define WavParser_h

#include <set>
#include <list>
#include <limits>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include "WavFormat.h"
#include "WavData.h"

// Reference Object
class WavParser {
 public:
  WavParser();
  explicit WavParser(std::string input);
  WavParser(std::string input, int target_track);
  ~WavParser();

  bool isWavFile();
  bool parse();
  void normalize();
  void debug_txt(std::string output);
  void debug_wav(std::string output);

  static void setWavHeader(std::ofstream *ofs, WavFormat format, long size_all);

  // accessor
  std::string getInput();
  void setInput(std::string input);
  void setTargetTracks(std::set<int> target_tracks);  
  void addTargetTrack(int target_track);
  std::set<int> getTargetTracks();
  WavFormat getFormat();
  std::list<WavData> getDataChunks();

 protected:
  std::string input;
  std::set<int> target_tracks;
  WavFormat format;
  std::list<WavData> data_chunks;

 private:
  WavParser(const WavParser& other);
  WavParser& operator=(const WavParser& other);
};

#endif
