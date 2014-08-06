#include "WavParser.h"

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
  long rest_size = 0, fmtChunkSize = 0;

  // fmt chunk
  ifs.seekg(sizeof(char)*4, ios_base::cur);
  ifs.read((char*)&rest_size, sizeof(long));
  ifs.seekg(sizeof(char)*8, ios_base::cur);
  ifs.read((char*)&fmtChunkSize, sizeof(long));
  {
    WavHeader header;
    ifs.read((char*)&header.wFormatTag, sizeof(short));
    ifs.read((char*)&header.wChannels, sizeof(short));
    ifs.read((char*)&header.dwSamplesPerSec, sizeof(long));
    ifs.read((char*)&header.dwAvgBytesPerSec, sizeof(long));
    ifs.read((char*)&header.wBlockAlign, sizeof(short));
    ifs.read((char*)&header.wBitsPerSamples, sizeof(short));
    for (size_t i=0; i<handlers.size(); i++) {
      handlers[i]->chunkHeader(header);
    }
    if (fmtChunkSize > 16) {
      short tmp_size = 0;
      ifs.read((char*)&tmp_size, sizeof(short));
      unsigned char* tmp_data = new unsigned char[tmp_size/sizeof(char)];
      ifs.read((char*)&tmp_data[0], tmp_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkHeaderAdditionalField(tmp_size, tmp_data);
      }
    }
  }
  rest_size -= 24;

  while (rest_size>sizeof(char)*4 && !ifs.eof()) {
    char tag[4] = {0};
    ifs.read((char*)tag, sizeof(char)*4);
    long chunk_size = 0;
    ifs.read((char*)&chunk_size, sizeof(long));
    if (WavData::isDataTag(tag)) {
      short* tmp_data = new short[chunk_size/sizeof(short)];
      ifs.read((char*)&tmp_data[0], chunk_size);
      WavData wav_data(tmp_data, chunk_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkData(wav_data);
      }
      delete[] tmp_data;
    } else if (WavData::isFactTag(tag)) {
      unsigned char* tmp_data = new unsigned char[chunk_size/sizeof(char)];
      ifs.read((char*)&tmp_data[0], chunk_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkFact(chunk_size, tmp_data);
      }
      delete[] tmp_data;
    } else if (WavData::isListTag(tag)) {
      unsigned char* tmp_data = new unsigned char[chunk_size/sizeof(char)];
      ifs.read((char*)&tmp_data[0], chunk_size);
      for (size_t i=0; i<handlers.size(); i++) {
        handlers[i]->chunkList(chunk_size, tmp_data);
      }
      delete[] tmp_data;
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
