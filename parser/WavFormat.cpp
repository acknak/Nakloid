#include "WavFormat.h"

const char WavFormat::riff[] = {'R','I','F','F'};
const char WavFormat::wave[] = {'W','A','V','E'};
const char WavFormat::fmt[] = {'f','m','t',' '};
const char WavFormat::fact[] = {'f','a','c','t'};
const char WavFormat::data[] = {'d','a','t','a'};
const char WavFormat::list[] = {'L','I','S','T'};

WavFormat& WavFormat::operator=(const WavFormat& other)
{
  if (this != &other) {
    wFormatTag = other.wFormatTag;
    wChannels = other.wChannels;
    dwSamplesPerSec = other.dwSamplesPerSec;
    dwAvgBytesPerSec = other.dwAvgBytesPerSec;
    wBlockAlign = other.wBlockAlign;
    wBitsPerSamples = other.wBitsPerSamples;
  }
  return *this;
}

void WavFormat::setDefaultValues()
{
  wFormatTag = 1; //linear
  wChannels = 1; //monoral
  dwSamplesPerSec = 44100;
  dwAvgBytesPerSec = 88200;
  wBlockAlign = 2;
  wBitsPerSamples = 16;
}

bool WavFormat::isTag(char* tag)
{
  bool is_tag = false;
  is_tag |= isRiffTag(tag);
  is_tag |= isWaveTag(tag);
  is_tag |= isFmtTag(tag);
  is_tag |= isDataTag(tag);
  is_tag |= isListTag(tag);
  return is_tag;
}

bool WavFormat::isRiffTag(char* tag)
{
  bool is_right_tag = true;

  for (int i=0; i<4; i++) {
    if (tag[i] != riff[i]) {
      is_right_tag = false;
      break;
    }
  }

  return is_right_tag;
}

bool WavFormat::isWaveTag(char* tag)
{
  bool is_right_tag = true;

  for (int i=0; i<4; i++) {
    if (tag[i] != wave[i]) {
      is_right_tag = false;
      break;
    }
  }

  return is_right_tag;
}

bool WavFormat::isFmtTag(char* tag)
{
  bool is_right_tag = true;

  for (int i=0; i<4; i++) {
    if (tag[i] != fmt[i]) {
      is_right_tag = false;
      break;
    }
  }

  return is_right_tag;
}

bool WavFormat::isFactTag(char* tag)
{
  bool is_right_tag = true;

  for (int i=0; i<4; i++) {
    if (tag[i] != fact[i]) {
      is_right_tag = false;
      break;
    }
  }

  return is_right_tag;
}

bool WavFormat::isDataTag(char* tag)
{
  bool is_right_tag = true;

  for (int i=0; i<4; i++) {
    if (tag[i] != data[i]) {
      is_right_tag = false;
      break;
    }
  }

  return is_right_tag;
}

bool WavFormat::isListTag(char* tag)
{
  bool is_right_tag = true;

  for (int i=0; i<4; i++) {
    if (tag[i] != list[i]) {
      is_right_tag = false;
      break;
    }
  }

  return is_right_tag;
}
