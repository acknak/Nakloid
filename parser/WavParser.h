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
class WavData;

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

  static void setWavFileFormat(std::ofstream *ofs, WavFormat format, long wav_size);
  static void setWavFile(std::ofstream *ofs, WavFormat format, const std::vector<double>* data);
  static void WavParser::sht2dbl(const std::vector<short>* from, std::vector<double>* to);
  static void WavParser::sht2dbl(const std::vector<short>::const_iterator from, std::vector<double>* to);
  static void WavParser::dbl2sht(const std::vector<double>* from, std::vector<short>* to);
  static void WavParser::dbl2sht(const std::vector<double>::const_iterator from, std::vector<short>* to);

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
