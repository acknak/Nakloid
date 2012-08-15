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
    cerr << "input is NULL" << endl;
    return false;
  }

  ifstream ifs(input.c_str(), ios::in | ios::binary);
  if (!ifs) {
    cerr << "file '" << input << "' cannot open" << endl;
    return false;
  }

  char tag[4];
  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavFormat::isRiffTag(tag)) {
    cerr << "error: file '" << input << "' is not RIFF format";
    return false;
  }

  ifs.seekg(sizeof(char)*4, ios_base::cur);

  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavFormat::isWaveTag(tag)) {
      cerr << "error: file '" << input << "' is not WAV" << endl;
    return false;
  }

  ifs.seekg(sizeof(char)*8, ios_base::cur);

  short format;
  ifs.read((char*)&format, sizeof(short));
  if (format != 1){
    cerr << "error: file '" << input << "' is not LinearPCM" << endl;
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

  // fmt chunk
  ifs.seekg(sizeof(char)*4, ios_base::cur);
  ifs.read((char*)&rest_size, sizeof(long));
  ifs.seekg(sizeof(char)*8, ios_base::cur);
  ifs.read((char*)&format.chunkSize, sizeof(long));
  ifs.read((char*)&format.wFormatTag, sizeof(short));
  ifs.read((char*)&format.wChannels, sizeof(short));
  ifs.read((char*)&format.dwSamplesPerSec, sizeof(long));
  ifs.read((char*)&format.dwAvgBytesPerSec, sizeof(long));
  ifs.read((char*)&format.wBlockAlign, sizeof(short));
  ifs.read((char*)&format.wBitsPerSamples, sizeof(short));
  ifs.seekg(sizeof(char)*(format.chunkSize - 16), ios_base::cur);

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
        cerr << endl << "*** unknown tag found at WavParser ***" << endl << endl;
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
    long length = (*it).getSize()/sizeof(short);
    double avg = 0.0, rate = 1.0;
    const short* fore_data = (*it).getData();
    short* aft_data = new short[length];

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
    (*it).setData(aft_data, (*it).getSize());

    delete[] aft_data;
  }
}

void WavParser::debug_txt(string output)
{
  if (&format == NULL || &data_chunks == NULL)
    return;

  ofstream ofs;
  ofs.open(output.c_str());

  ofs << "---------- format chunk ----------" << endl << endl
      << setw(20) << "chunkSize: " << setw(8) << format.chunkSize << endl
      << setw(20) << "wFormatTag: " << setw(8) << format.wFormatTag << endl
      << setw(20) << "wChannels: " << setw(8) << format.wChannels << endl
      << setw(20) << "dwSamplesPerSec: " << setw(8) << format.dwSamplesPerSec << endl
      << setw(20) << "dwAvgBytesPerSec: " << setw(8) << format.dwAvgBytesPerSec << endl
      << setw(20) << "wBlockAlign: " << setw(8) << format.wBlockAlign << endl
      << setw(20) << "wBitsPerSamples: " << setw(8) << format.wBitsPerSamples << endl << endl << endl;

  for (list<WavData>::iterator it=data_chunks.begin(); it!=data_chunks.end(); ++it) {
    ofs << "---------- data chunk ----------" << endl << endl
        << setw(20) << "chunkSize: " << setw(8) << (*it).getSize() << endl << endl
        << "wavformData" << endl;
    const short* tmp_wav_data = (*it).getData();
    for (long i=0; i<(*it).getSize()/sizeof(short); i++)
      ofs << setw(11) << tmp_wav_data[i] << endl;
  }
}

void WavParser::debug_wav(string output)
{
  cout << "start wav output" << endl;
  long size_all = 28;
  for (list<WavData>::iterator it=data_chunks.begin(); it!=data_chunks.end(); ++it)
    size_all += (*it).getSize() + 8;

  ofstream ofs;
  ofs.open(output.c_str(), ios_base::out|ios_base::trunc|ios_base::binary);
  setWavHeader(&ofs, format, size_all);

  for (list<WavData>::iterator it=data_chunks.begin(); it!=data_chunks.end(); ++it) {
    ofs.write((char*)WavFormat::data, sizeof(char)*4);
    long size = (*it).getSize();
    ofs.write((char*)&size, sizeof(long));
    ofs.write((char*)(*it).getData(), (*it).getSize());
  }

  ofs.close();
}

void WavParser::setWavHeader(ofstream *ofs, WavFormat format, long size_all) {
  ofs->write((char*)WavFormat::riff, sizeof(char)*4);
  ofs->write((char*)&size_all, sizeof(long));
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
