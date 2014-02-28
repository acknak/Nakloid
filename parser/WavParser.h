#ifndef WavParser_h
#define WavParser_h

#include <fstream>
#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>
#include <boost/filesystem/fstream.hpp>
#include "WavData.h"
#include "WavFormat.h"
class WavData;

class WavParser {
 public:
  WavParser();
  explicit WavParser(const std::wstring& input);
  WavParser(const std::wstring& input, int target_track);
  virtual ~WavParser();

  bool isWavFile() const;
  bool parse();

  static void setWavFileFormat(boost::filesystem::ofstream* const  ofs, const WavFormat& format, long wav_size);
  static void setWavFile(boost::filesystem::ofstream* const ofs, const WavFormat& format, const std::vector<double>* data);
  static void sht2dbl(const std::vector<short>& from, std::vector<double>* to);
  static void sht2dbl(const std::vector<short>::const_iterator from, std::vector<double>* to);
  static void dbl2sht(const std::vector<double>& from, std::vector<short>* to);
  static void dbl2sht(const std::vector<double>::const_iterator from, std::vector<short>* to);

  // accessor
  const std::wstring& getInput() const;
  void setInput(const std::wstring& input);
  void setTargetTracks(const std::set<int>& target_tracks);  
  void addTargetTrack(int target_track);
  const std::set<int>& getTargetTracks() const;
  const WavFormat& getFormat() const;
  const std::vector<WavData>& getDataChunks() const;

 protected:
  std::wstring input;
  std::set<int> target_tracks;
  WavFormat format;
  std::vector<WavData> data_chunks;

 private:
  WavParser(const WavParser& other);
  WavParser& operator=(const WavParser& other);
};

#endif
