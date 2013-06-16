#include "WavParser.h"

using namespace std;

WavParser::WavParser(){}

WavParser::WavParser(string input)
{
  setInput(input);
}

WavParser::WavParser(string input, int target_track)
{
  setInput(input);
  addTargetTrack(target_track);
}

WavParser::~WavParser(){}

string WavParser::getInput()
{
  return input;
}

void WavParser::setInput(string input)
{
  this->input = input;
}

void WavParser::setTargetTracks(set<int> target_tracks)
{
  this->target_tracks = target_tracks;
}

void WavParser::addTargetTrack(int target_track)
{
  this->target_tracks.insert(target_track);
}

set<int> WavParser::getTargetTracks()
{
  return target_tracks;
}

WavFormat WavParser::getFormat()
{
  return format;
}

list<WavData> WavParser::getDataChunks()
{
  return data_chunks;
}

bool WavParser::isWavFile()
{
  if (input.empty()) {
    cerr << "[WavParser::isWavFile] input is NULL" << endl;
    return false;
  }

  ifstream ifs(input.c_str(), ios::in | ios::binary);
  if (!ifs) {
    cerr << "[WavParser::isWavFile] file '" << input << "' cannot open" << endl;
    return false;
  }

  char tag[4];
  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavFormat::isRiffTag(tag)) {
    cerr << "[WavParser::isWavFile] file '" << input << "' is not RIFF format";
    return false;
  }

  ifs.seekg(sizeof(char)*4, ios_base::cur);

  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavFormat::isWaveTag(tag)) {
      cerr << "[WavParser::isWavFile] file '" << input << "' is not WAV" << endl;
    return false;
  }

  ifs.seekg(sizeof(char)*8, ios_base::cur);

  short format;
  ifs.read((char*)&format, sizeof(short));
  if (format != 1){
    cerr << "[WavParser::isWavFile] file '" << input << "' is not LinearPCM" << endl;
    return false;
  }

  return true;
}

bool WavParser::parse()
{
  if (!isWavFile())
    return false;

  if (!data_chunks.empty())
    data_chunks.clear();

  ifstream ifs(input.c_str(), ios::in | ios::binary);
  long rest_size = 0;
  long fmtChunkSize = 0;

  // fmt chunk
  ifs.seekg(sizeof(char)*4, ios_base::cur);
  ifs.read((char*)&rest_size, sizeof(long));
  ifs.seekg(sizeof(char)*8, ios_base::cur);
  ifs.read((char*)&fmtChunkSize, sizeof(long));
  ifs.read((char*)&format.wFormatTag, sizeof(short));
  ifs.read((char*)&format.wChannels, sizeof(short));
  ifs.read((char*)&format.dwSamplesPerSec, sizeof(long));
  ifs.read((char*)&format.dwAvgBytesPerSec, sizeof(long));
  ifs.read((char*)&format.wBlockAlign, sizeof(short));
  ifs.read((char*)&format.wBitsPerSamples, sizeof(short));
  ifs.seekg(sizeof(char)*(fmtChunkSize - 16), ios_base::cur);

  // data chunk
  rest_size -= 24;
  set<int>::iterator it = target_tracks.begin();
  int track = 0;
  while (rest_size > 0) {
    // check is data chunk
    char tag[4];
    ifs.read((char*)tag, sizeof(char)*4);

    // get chunk size
    long chunk_size;
    ifs.read((char*)&chunk_size, sizeof(long));

    if (WavFormat::isDataTag(tag) && track == (*it)) {
      // data chunk
      short* data = new short[chunk_size/sizeof(short)];

      for (int j=0; j<chunk_size/sizeof(short); j++)
        ifs.read((char*)&data[j], sizeof(short));

      WavData wav_data;
      wav_data.setData(data, chunk_size);
      data_chunks.push_back(wav_data);
      delete[] data;

      if (++it == target_tracks.end())
        break;
      ++track;
    } else {
      // other chunk
      if (WavFormat::isTag(tag))
        cerr << endl << "[WavParser::parse] unknown tag found at WavParser" << endl << endl;
      ifs.seekg(chunk_size, ios_base::cur);
    }

    rest_size -= chunk_size + 8;
  }
  return true;
}

void WavParser::normalize()
{
  if (data_chunks.empty())
    return;

  for (list<WavData>::iterator it=data_chunks.begin(); it!=data_chunks.end(); ++it) {
    long max = numeric_limits<short>::min();
    long min = numeric_limits<short>::max();
    long length = it->getSize();
    double avg = 0.0, rate = 1.0;
    vector<short> fore_data(it->getWavData());
    vector<short> aft_data(fore_data.size(), 0);

    for (int i=0; i<length; i++) {
      short tmp_data = fore_data[i];
      if (tmp_data > max)
        max = tmp_data;
      if (tmp_data < min)
        min = tmp_data;
      avg = avg/(i+1)*i + ((double)tmp_data/(i+1));
    }
    if (avg-min < max-avg)
      rate = (numeric_limits<short>::max()*0.5) / (max-avg);
    else
      rate = (numeric_limits<short>::min()*0.5) / (avg-min);
    for (int i=0; i<length; i++)
      aft_data[i] = (fore_data[i]-avg) * rate;
    it->setData(&aft_data[0], aft_data.size());
  }
}

void WavParser::setWavFileFormat(ofstream *ofs, WavFormat format, long wav_size) {
  ofs->write((char*)WavFormat::riff, sizeof(char)*4);
  ofs->write((char*)&wav_size, sizeof(long));
  ofs->write((char*)WavFormat::wave, sizeof(char)*4);
  ofs->write((char*)WavFormat::fmt, sizeof(char)*4);
  ofs->write((char*)&(format.chunkSize), sizeof(long));
  ofs->write((char*)&(format.wFormatTag), sizeof(short));
  ofs->write((char*)&(format.wChannels), sizeof(short));
  ofs->write((char*)&(format.dwSamplesPerSec), sizeof(long));
  ofs->write((char*)&(format.dwAvgBytesPerSec), sizeof(long));
  ofs->write((char*)&(format.wBlockAlign), sizeof(short));
  ofs->write((char*)&(format.wBitsPerSamples), sizeof(short));
}

void WavParser::setWavFile(ofstream *ofs, WavFormat format, const vector<double>* data) {
  vector<short> output_data(data->size(), 0);

  dbl2sht(data, &output_data);
  long data_chunk_size = output_data.size() * sizeof(short);
  long wav_size = data_chunk_size + format.chunkSize + 12;

  setWavFileFormat(ofs, format, wav_size);

  ofs->write((char*)WavFormat::data, sizeof(char)*4);
  ofs->write((char*)&data_chunk_size, sizeof(long));
  ofs->write((char*)&output_data[0], data_chunk_size);
}

void WavParser::sht2dbl(const vector<short>* from, vector<double>* to)
{
  if (from->size() != to->size()) {
    cerr << "[WavParser::sht2dbl] 'from' size different from 'to' size" << endl;
    return;
  }
  sht2dbl(from->begin(), to);
}

void WavParser::sht2dbl(const vector<short>::const_iterator from, vector<double>* to)
{
  for (int i=0; i<to->size(); i++) {
    to->at(i) = *(from+i) / 32768.0;
  }
}

void WavParser::dbl2sht(const vector<double>* from, vector<short>* to)
{
  if (from->size() != to->size()) {
    cerr << "[WavParser::dbl2sht] 'from' size different from 'to' size" << endl;
    return;
  }
  dbl2sht(from->begin(), to);
}

void WavParser::dbl2sht(const vector<double>::const_iterator from, vector<short>* to)
{
  for (int i=0; i<to->size(); i++) {
    to->at(i) = *(from+i) * 32767;
  }
}
