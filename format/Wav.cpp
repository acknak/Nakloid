#include "Wav.h"

#include <cstdint>
#include <vector>
#include <boost/filesystem/fstream.hpp>

using namespace std;

const char WavHeader::tag_riff[] = {'R','I','F','F'};
const char WavHeader::tag_wave[] = {'W','A','V','E'};
const char WavHeader::tag_fmt_[] = {'f','m','t',' '};
const char WavData::tag_fact[] = {'f','a','c','t'};
const char WavData::tag_data[] = {'d','a','t','a'};
const char WavData::tag_list[] = {'L','I','S','T'};

WavHeader& WavHeader::operator=(const WavHeader& other)
{
  if (this != &other) {
    wFormatTag = other.wFormatTag;
    wChannels = other.wChannels;
    dwSamplesPerSec = other.dwSamplesPerSec;
    dwAvgBytesPerSec = other.dwAvgBytesPerSec;
    wBlockAlign = other.wBitsPerSamples;
  }
  return *this;
}

bool WavHeader::operator==(const WavHeader& other) const
{
  return wFormatTag==other.wFormatTag && wChannels==wChannels && dwSamplesPerSec==dwSamplesPerSec
    && dwAvgBytesPerSec==other.dwAvgBytesPerSec && wBlockAlign==other.wBlockAlign && wBitsPerSamples==wBitsPerSamples;
}

bool WavHeader::operator!=(const WavHeader& other) const
{
  return !(*this == other);
}

bool WavHeader::isRiffTag(const char* const tag)
{
  for (size_t i=0; i<4; i++) {
    if (tag[i] != tag_riff[i]) {
      return false;
    }
  }
  return true;
}

bool WavHeader::isWaveTag(const char* const tag)
{
  for (size_t i=0; i<4; i++) {
    if (tag[i] != tag_wave[i]) {
      return false;
    }
  }
  return true;
}

bool WavHeader::isFmtTag(const char* const tag)
{
  for (size_t i=0; i<4; i++) {
    if (tag[i] != tag_fmt_[i]) {
      return false;
    }
  }
  return true;
}

WavData::WavData(const short* const data, long chunk_size)
{
  setData(data, chunk_size);
}

WavData& WavData::operator=(const WavData& other)
{
  if (this != &other) {
    data = other.getData();
  }
  return *this;
}

bool WavData::operator==(const WavData& other) const
{
  return data==other.data;
}

bool WavData::operator!=(const WavData& other) const
{
  return data!=other.data;
}

void WavData::clear()
{
  // release memory
  vector<double>(data).swap(data);
}

vector<short> WavData::getDataForWavFile() const
{
  vector<int16_t> tmp(data.size(), 0);
  for (size_t i=0; i<data.size(); i++) {
    tmp[i] = data[i] * 32767;
  }
  return tmp;
}

const vector<double>& WavData::getData() const
{
  return data;
}

void WavData::setData(const int16_t* const data, uint32_t chunk_size)
{
  if (data==0 || chunk_size==0) {
    cerr << "[WavData::setData] can't assign null data";
    return;
  }
  this->data.assign(chunk_size/sizeof(int16_t), 0);
  for (size_t i=0; i<this->data.size(); i++) {
    this->data[i] = data[i]/32768.0;
  }
}

void WavData::setData(const vector<double>& data)
{
  this->data = data;
}

long WavData::getSize() const
{
  return data.size();
}

long WavData::getSizeForWavFile() const
{
  return data.size()*sizeof(int16_t);
}

double WavData::getRMS() const
{
  double rms = 0.0;
  for (size_t i=0; i<data.size(); i++) {
    rms += pow(data[i], 2) / data.size();
  }
  return sqrt(rms);
}

bool WavData::isFactTag(const char* const tag)
{
  for (size_t i=0; i<4; i++) {
    if (tag[i] != tag_fact[i]) {
      return false;
    }
  }
  return true;
}

bool WavData::isDataTag(const char* const tag)
{
  for (size_t i=0; i<4; i++) {
    if (tag[i] != tag_data[i]) {
      return false;
    }
  }
  return true;
}

bool WavData::isListTag(const char* const tag)
{
  for (size_t i=0; i<4; i++) {
    if (tag[i] != tag_list[i]) {
      return false;
    }
  }
  return true;
}

Wav& Wav::operator=(const Wav& other)
{
  if (this != &other) {
    header = other.header;
    data = other.data;
  }
  return *this;
}

bool Wav::operator==(const Wav& other) const
{
  return header==other.header && data==data;
}

bool Wav::operator!=(const Wav& other) const
{
  return !(*this == other);
}

void Wav::save(const boost::filesystem::path& path)
{
  const vector<int16_t>& output_data = data.getDataForWavFile();
  long data_chunk_size = output_data.size() * sizeof(int16_t);
  long wav_size = data_chunk_size + WavHeader::const_chunk_size + 12;

  boost::filesystem::ofstream ofs(path, ios_base::binary);
  ofs.write((char*)WavHeader::tag_riff, sizeof(char)*4);
  ofs.write((char*)&wav_size, sizeof(uint32_t));
  ofs.write((char*)WavHeader::tag_wave, sizeof(char)*4);
  ofs.write((char*)WavHeader::tag_fmt_, sizeof(char)*4);
  uint32_t const_chunk_size = WavHeader::const_chunk_size;
  ofs.write((char*)&(const_chunk_size), sizeof(uint32_t));
  ofs.write((char*)&(header.wFormatTag), sizeof(uint16_t));
  ofs.write((char*)&(header.wChannels), sizeof(uint16_t));
  ofs.write((char*)&(header.dwSamplesPerSec), sizeof(uint32_t));
  ofs.write((char*)&(header.dwAvgBytesPerSec), sizeof(uint32_t));
  ofs.write((char*)&(header.wBlockAlign), sizeof(uint16_t));
  ofs.write((char*)&(header.wBitsPerSamples), sizeof(uint16_t));
  ofs.write((char*)WavData::tag_data, sizeof(char)*4);
  ofs.write((char*)&data_chunk_size, sizeof(uint32_t));
  ofs.write((char*)&output_data[0], data_chunk_size);
}

void Wav::clear()
{
  header = WavHeader();
  data.clear();
}
