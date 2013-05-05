#include "BaseWavsFileIO.h"

using namespace std;

bool bwc::isBaseWavsContainerFile(string filename)
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
  if (chunkSize != 16+BaseWavsFormat::wAdditionalSize+sizeof(short))
    return false;

  // check format ID
  unsigned short wFormatTag = 0;
  ifs.seekg(20, ios::beg);
  ifs.read((char*)&wFormatTag, sizeof(short));
  if (wFormatTag != BaseWavsFormat::BaseWavsFormatTag)
    return false;

  return true;
}

BaseWavsContainer bwc::get(string filename)
{
  BaseWavsContainer bwc;
  if (!isBaseWavsContainerFile(filename)) {
    return bwc;
  } else {
    ifstream ifs(filename.c_str(), ios_base::binary);

    // fmt chunk
    ifs.seekg(16, ios_base::beg);
    ifs.read((char*)&bwc.format.chunkSize, sizeof(long));
    ifs.read((char*)&bwc.format.wFormatTag, sizeof(short));
    ifs.read((char*)&bwc.format.wChannels, sizeof(short));
    ifs.read((char*)&bwc.format.dwSamplesPerSec, sizeof(long));
    ifs.read((char*)&bwc.format.dwAvgBytesPerSec, sizeof(long));
    ifs.read((char*)&bwc.format.wBlockAlign, sizeof(short));
    ifs.read((char*)&bwc.format.wBitsPerSamples, sizeof(short));
    ifs.seekg(2, ios_base::cur);
    ifs.read((char*)&bwc.format.wLobeSize, sizeof(short));
    ifs.read((char*)&bwc.format.dwRepeatStart, sizeof(long));
    ifs.read((char*)&bwc.format.wF0, sizeof(double));

    list<BaseWav> base_wav_list;
    while(!ifs.eof()) {
      BaseWav tmp_base_wav;
      char tag[4];
      long chunkSize;

      // fact chunk
      ifs.read((char*)&tag, sizeof(char)*4);
      ifs.read((char*)&chunkSize, sizeof(long));
      if (!WavFormat::isFactTag(tag) || chunkSize!=BaseWavFact::chunkSize) {
        if (ifs.eof())
          break;
        ifs.seekg(chunkSize, ios::cur);
        cerr << "[BaseWavsFileIO::get] fact chunk not found" << endl;
        continue;
      } else {
        ifs.read((char*)&(tmp_base_wav.fact.dwPitchLeft), sizeof(long));
        ifs.read((char*)&(tmp_base_wav.fact.dwPitchRight), sizeof(long));
        ifs.read((char*)&(tmp_base_wav.fact.dwPosition), sizeof(long));
      }

      // data chunk
      ifs.read((char*)&tag, sizeof(char)*4);
      ifs.read((char*)&chunkSize, sizeof(long));
      if (!WavFormat::isDataTag(tag)) {
        ifs.seekg(chunkSize, ios::cur);
        cerr << "[BaseWavsFileIO::get] data chunk not found" << endl;
        continue;
      } else {
        vector<short> tmp_wav_data_vector(chunkSize/sizeof(short), 0);
        ifs.read((char*)&(tmp_wav_data_vector[0]), chunkSize);
        WavData tmp_wav_data(tmp_wav_data_vector);
        tmp_base_wav.data = tmp_wav_data;
      }

      base_wav_list.push_back(tmp_base_wav);
    }

    bwc.base_wavs.assign(base_wav_list.begin(), base_wav_list.end());
  }

  return bwc;
}

bool bwc::set(string filename, BaseWavsContainer *bwc)
{
  short wAdditionalSize = BaseWavsFormat::wAdditionalSize;
  long size_all = 28 + wAdditionalSize + sizeof(short);
  ofstream ofs(filename.c_str(), ios_base::trunc|ios_base::binary);
  WavParser::setWavHeader(&ofs, bwc->format, size_all);

  ofs.write((char*)&(wAdditionalSize), sizeof(short));
  ofs.write((char*)&(bwc->format.wLobeSize), sizeof(short));
  ofs.write((char*)&(bwc->format.dwRepeatStart), sizeof(long));
  ofs.write((char*)&(bwc->format.wF0), sizeof(double));

  // fact chunk & data chunk
  vector<BaseWav> base_wavs = bwc->base_wavs;
  for(vector<BaseWav>::iterator it=bwc->base_wavs.begin(); it!=bwc->base_wavs.end(); ++it) {
    long factChunkSize = BaseWavFact::chunkSize;
    long dataChunkSize = it->data.getSize();
    ofs.write((char*)WavFormat::fact, sizeof(char)*4);
    ofs.write((char*)&(factChunkSize), sizeof(long));
    ofs.write((char*)&(it->fact.dwPitchLeft), sizeof(long));
    ofs.write((char*)&(it->fact.dwPitchRight), sizeof(long));
    ofs.write((char*)&(it->fact.dwPosition), sizeof(long));
    ofs.write((char*)WavFormat::data, sizeof(char)*4);
    ofs.write((char*)&(dataChunkSize), sizeof(long));
    ofs.write((char*)it->data.getData(), it->data.getSize());
    size_all += factChunkSize + sizeof(char)*4 + sizeof(long)
      + dataChunkSize + sizeof(char)*4 + sizeof(long);
  }
  ofs.seekp(4, ios::beg);
  ofs.write((char*)&(size_all), sizeof(long));

  return true;
}
