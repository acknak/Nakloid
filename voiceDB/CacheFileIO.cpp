#include "CacheFileIO.h"

using namespace std;
using namespace uw;

bool uw::isUwcFile(string filename)
{
  ifstream ifs(filename.c_str(), ios_base::binary);

  // check RIFF tag
  char tag[4];
  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavFormat::isRiffTag(tag))
    return false;

  // check WAVE tag
  ifs.seekg(8, ios_base::beg);
  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavFormat::isWaveTag(tag))
    return false;

  // check fmt chunkSize
  long chunkSize = 0;
  ifs.seekg(16, ios::beg);
  ifs.read((char*)&chunkSize, sizeof(long));
  if (chunkSize != UnitWaveformFormat::chunkSize)
    return false;

  // check format ID
  unsigned short wFormatTag = 0;
  ifs.seekg(20, ios::beg);
  ifs.read((char*)&wFormatTag, sizeof(short));
  if (wFormatTag != UnitWaveformFormat::UnitWaveformFormatTag)
    return false;

  return true;
}

UnitWaveformContainer uw::load(string filename)
{
  UnitWaveformContainer uwc;
  if (!uw::isUwcFile(filename)) {
    return uwc;
  } else {
    ifstream ifs(filename.c_str(), ios_base::binary);

    // fmt chunk
    ifs.seekg(20, ios_base::beg);
    ifs.read((char*)&uwc.format.wFormatTag, sizeof(short));
    ifs.read((char*)&uwc.format.wChannels, sizeof(short));
    ifs.read((char*)&uwc.format.dwSamplesPerSec, sizeof(long));
    ifs.read((char*)&uwc.format.dwAvgBytesPerSec, sizeof(long));
    ifs.read((char*)&uwc.format.wBlockAlign, sizeof(short));
    ifs.read((char*)&uwc.format.wBitsPerSamples, sizeof(short));
    ifs.seekg(2, ios_base::cur);
    ifs.read((char*)&uwc.format.wLobeSize, sizeof(short));
    ifs.read((char*)&uwc.format.dwRepeatStart, sizeof(long));
    ifs.read((char*)&uwc.format.wF0, sizeof(double));

    list<UnitWaveform> unit_waveforms;
    while(!ifs.eof()) {
      UnitWaveform tmp_unit_waveform;
      char tag[4];
      long chunkSize;

      // fact chunk
      ifs.read((char*)&tag, sizeof(char)*4);
      ifs.read((char*)&chunkSize, sizeof(long));
      if (!WavFormat::isFactTag(tag) || chunkSize!=UnitWaveformFact::chunkSize) {
        if (ifs.eof())
          break;
        ifs.seekg(chunkSize, ios::cur);
        cerr << "[UnitWaveformFileIO::get] fact chunk not found" << endl;
        continue;
      } else {
        ifs.read((char*)&(tmp_unit_waveform.fact.dwPitchLeft), sizeof(long));
        ifs.read((char*)&(tmp_unit_waveform.fact.dwPitchRight), sizeof(long));
        ifs.read((char*)&(tmp_unit_waveform.fact.dwPosition), sizeof(long));
      }

      // data chunk
      ifs.read((char*)&tag, sizeof(char)*4);
      ifs.read((char*)&chunkSize, sizeof(long));
      if (!WavFormat::isDataTag(tag)) {
        ifs.seekg(chunkSize, ios::cur);
        cerr << "[UnitWaveformFileIO::get] data chunk not found" << endl;
        continue;
      } else {
        vector<double> tmp_wav_data_vector(chunkSize/sizeof(short), 0);
        ifs.read((char*)&(tmp_wav_data_vector[0]), chunkSize);
        WavData tmp_wav_data(tmp_wav_data_vector);
        tmp_unit_waveform.data = tmp_wav_data;
      }

      unit_waveforms.push_back(tmp_unit_waveform);
    }

    uwc.unit_waveforms.assign(unit_waveforms.begin(), unit_waveforms.end());
  }

  return uwc;
}

bool uw::save(string filename, UnitWaveformContainer *uwc)
{
  short wAdditionalSize = UnitWaveformFormat::wAdditionalSize;
  long size_all = wAdditionalSize + sizeof(short);
  ofstream ofs(filename.c_str(), ios_base::trunc|ios_base::binary);
  WavParser::setWavFileFormat(&ofs, uwc->format, size_all);

  ofs.write((char*)&(wAdditionalSize), sizeof(short));
  ofs.write((char*)&(uwc->format.wLobeSize), sizeof(short));
  ofs.write((char*)&(uwc->format.dwRepeatStart), sizeof(long));
  ofs.write((char*)&(uwc->format.wF0), sizeof(double));

  // fact chunk & data chunk
  vector<UnitWaveform> unit_waveforms = uwc->unit_waveforms;
  for(vector<UnitWaveform>::iterator it=uwc->unit_waveforms.begin(); it!=uwc->unit_waveforms.end(); ++it) {
    vector<short> cache_data(it->data.getSize(), 0);
    WavParser::dbl2sht(it->data.getDataIterator(), &cache_data);
    long dataChunkSize = it->data.getSize()*sizeof(short);
    ofs.write((char*)WavFormat::fact, sizeof(char)*4);
    ofs.write((char*)&UnitWaveformFact::chunkSize, sizeof(long));
    ofs.write((char*)&(it->fact.dwPitchLeft), sizeof(long));
    ofs.write((char*)&(it->fact.dwPitchRight), sizeof(long));
    ofs.write((char*)&(it->fact.dwPosition), sizeof(long));
    ofs.write((char*)WavFormat::data, sizeof(char)*4);
    ofs.write((char*)&(dataChunkSize), sizeof(long));
    ofs.write((char*)&it->data.getData()[0], dataChunkSize);
    size_all += UnitWaveformFact::chunkSize + sizeof(char)*4 + sizeof(long)
      + dataChunkSize + sizeof(char)*4 + sizeof(long);
  }
  ofs.seekp(4, ios::beg);
  ofs.write((char*)&(size_all), sizeof(long));

  return true;
}
