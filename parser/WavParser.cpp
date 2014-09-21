#include "WavParser.h"

#include "WavHandler.h"

#include <cstdint>
#include <boost/filesystem/fstream.hpp>

using namespace std;

WavParser::WavParser(){}

WavParser::WavParser(const boost::filesystem::path& path_input, WavHandler* const handler):path_input(path_input)
{
  addWavHandler(handler);
}

WavParser::~WavParser(){}

bool WavParser::isWavFile() const
{
  if (path_input.empty()) {
    cerr << "[WavParser::isWavFile] input is NULL" << endl;
    return false;
  }

  boost::filesystem::ifstream ifs(path_input, ios::in | ios::binary);
  if (!ifs) {
    wcerr << L"[WavParser::isWavFile] file '" << path_input << L"' cannot open" << endl;
    return false;
  }

  char tag[4];
  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavHeader::isRiffTag(tag)) {
    wcerr << L"[WavParser::isWavFile] file '" << path_input << L"' is not RIFF format";
    return false;
  }

  ifs.seekg(sizeof(char)*4, ios_base::cur);

  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavHeader::isWaveTag(tag)) {
      wcerr << L"[WavParser::isWavFile] file '" << path_input << L"' is not WAV" << endl;
    return false;
  }

  return true;
}

bool WavParser::parse()
{
  if (!isWavFile() || handlers.empty()) {
    return false;
  }

  boost::filesystem::ifstream ifs(path_input, ios::in | ios::binary);
  uint32_t rest_size = 0, fmtChunkSize = 0;

  // fmt chunk
  ifs.seekg(sizeof(char)*4, ios_base::cur);
  ifs.read((char*)&rest_size, sizeof(uint32_t));
  ifs.seekg(sizeof(char)*8, ios_base::cur);
  ifs.read((char*)&fmtChunkSize, sizeof(uint32_t));
  {
    WavHeader header;
    ifs.read((char*)&header.wFormatTag, sizeof(uint16_t));
    ifs.read((char*)&header.wChannels, sizeof(uint16_t));
    ifs.read((char*)&header.dwSamplesPerSec, sizeof(uint32_t));
    ifs.read((char*)&header.dwAvgBytesPerSec, sizeof(uint32_t));
    ifs.read((char*)&header.wBlockAlign, sizeof(uint16_t));
    ifs.read((char*)&header.wBitsPerSamples, sizeof(uint16_t));
    for (size_t i=0; i<handlers.size(); i++) {
      handlers[i]->chunkHeader(header);
    }
    if (fmtChunkSize > 16) {
      uint16_t tmp_size = 0;
      ifs.read((char*)&tmp_size, sizeof(uint16_t));
      uint8_t* tmp_data = new uint8_t[tmp_size/sizeof(uint8_t)];
      ifs.read((char*)&tmp_data[0], tmp_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkHeaderAdditionalField(tmp_size, tmp_data);
      }
    }
  }
  rest_size -= 24;

  while (rest_size>(uint32_t)sizeof(uint8_t)*4 && !ifs.eof()) {
    char tag[4] = {0};
    ifs.read((char*)tag, sizeof(char)*4);
    long chunk_size = 0;
    ifs.read((char*)&chunk_size, sizeof(uint32_t));
    if (WavData::isDataTag(tag)) {
      vector<int16_t> tmp_data(chunk_size/sizeof(int16_t));
      ifs.read((char*)tmp_data.data(), chunk_size);
      WavData wav_data(tmp_data.data(), chunk_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkData(wav_data);
      }
    } else if (WavData::isFactTag(tag)) {
      vector<uint8_t> tmp_data(chunk_size/sizeof(uint8_t));
      ifs.read((char*)tmp_data.data(), chunk_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkFact(chunk_size, tmp_data.data());
      }
    } else if (WavData::isListTag(tag)) {
      vector<uint8_t> tmp_data(chunk_size/sizeof(uint8_t));
      ifs.read((char*)tmp_data.data(), chunk_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkList(chunk_size, tmp_data.data());
      }
    } else {
      wcerr << L"[WavParser::parse] unknown tag found at " << path_input << endl;
      ifs.seekg(chunk_size, ios_base::cur);
    }
    rest_size -= chunk_size + 8;
  }
  return true;
}

/*
 * accessor
 */
const boost::filesystem::path& WavParser::getPathInput() const
{
  return path_input;
}

void WavParser::setPathInput(const boost::filesystem::path& path_input)
{
  this->path_input = path_input;
}

void WavParser::setWavHandler(const vector<WavHandler*>& handlers)
{
  this->handlers.clear();
  this->handlers = handlers;
}

void WavParser::addWavHandler(WavHandler* const handler)
{
  handlers.push_back(handler);
}
