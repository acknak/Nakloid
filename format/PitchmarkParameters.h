#ifndef PitchmarkParameters_h
#define PitchmarkParameters_h

#include <vector>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

class PitchmarkParameters {
 public:
  PitchmarkParameters():filename(L""),sub_fade_start(0),base_pitch(0),pitchmark_points(0){}
  bool operator==(const PitchmarkParameters& other) const;
  bool operator!=(const PitchmarkParameters& other) const;

  std::wstring filename;
  long sub_fade_start;
  long base_pitch;
  std::vector<long> pitchmark_points;

  std::wstring base_vowel_wav_filename;
  long base_vowel_wav_from;
  long base_vowel_wav_to;

  std::wstring prefix_vowel_wav_filename;
  long prefix_vowel_wav_from;
  long prefix_vowel_wav_to;

  void load(const boost::filesystem::wpath& path);
  void save(const boost::filesystem::wpath& path);
};

#endif
