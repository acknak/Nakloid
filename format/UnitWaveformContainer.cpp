#include "UnitWaveformContainer.h"

using namespace std;

bool UnitWaveformHeader::operator==(const UnitWaveformHeader& other) const
{
  return wLobeSize==other.wLobeSize && dwRepeatStart==other.dwRepeatStart && wF0==other.wF0;
}

bool UnitWaveformHeader::operator!=(const UnitWaveformHeader& other) const
{
  return !((*this)==other);
}

bool UnitWaveform::operator==(const UnitWaveform& other) const
{
  return dwPitchLeft==other.dwPitchLeft && dwPitchRight==other.dwPitchRight && dwPosition==other.dwPosition && data==other.data;
}

bool UnitWaveform::operator!=(const UnitWaveform& other) const
{
  return !((*this)==other);
}

bool UnitWaveformContainer::operator==(const UnitWaveformContainer& other) const
{
  return header==other.header && unit_waveforms==other.unit_waveforms;
}

bool UnitWaveformContainer::operator!=(const UnitWaveformContainer& other) const
{
  return !((*this)==other);
}

void UnitWaveformContainer::save(const boost::filesystem::path& path_uwc)
{
  long size_all = 0;

  boost::filesystem::ofstream ofs(path_uwc, ios_base::trunc|ios_base::binary);
  ofs.write((char*)WavHeader::tag_riff, sizeof(char)*4);
  ofs.write((char*)&size_all, sizeof(long));
  ofs.write((char*)WavHeader::tag_wave, sizeof(char)*4);
  ofs.write((char*)WavHeader::tag_fmt_, sizeof(char)*4);
  ofs.write((char*)&(UnitWaveformHeader::const_chunk_size), sizeof(long));
  ofs.write((char*)&(header.wFormatTag), sizeof(short));
  ofs.write((char*)&(header.wChannels), sizeof(short));
  ofs.write((char*)&(header.dwSamplesPerSec), sizeof(long));
  ofs.write((char*)&(header.dwAvgBytesPerSec), sizeof(long));
  ofs.write((char*)&(header.wBlockAlign), sizeof(short));
  ofs.write((char*)&(header.wBitsPerSamples), sizeof(short));
  ofs.write((char*)&(UnitWaveformHeader::wAdditionalSize), sizeof(short));
  ofs.write((char*)&(header.wLobeSize), sizeof(short));
  ofs.write((char*)&(header.dwRepeatStart), sizeof(long));
  ofs.write((char*)&(header.wF0), sizeof(float));

  size_all += 16 + UnitWaveformHeader::wAdditionalSize;

  // fact chunk & data chunk
  for (size_t i=0; i<unit_waveforms.size(); i++) {
    vector<short> data_short = unit_waveforms[i].data.getDataForWavFile();
    long dataChunkSize = data_short.size()*sizeof(short);

    ofs.write((char*)WavData::tag_fact, sizeof(char)*4);
    ofs.write((char*)&UnitWaveform::chunkSize, sizeof(long));
    ofs.write((char*)&(unit_waveforms[i].dwPitchLeft), sizeof(long));
    ofs.write((char*)&(unit_waveforms[i].dwPitchRight), sizeof(long));
    ofs.write((char*)&(unit_waveforms[i].dwPosition), sizeof(long));
    size_all += sizeof(char)*4 + sizeof(long) + UnitWaveform::chunkSize;

    ofs.write((char*)WavData::tag_data, sizeof(char)*4);
    ofs.write((char*)&dataChunkSize, sizeof(long));
    ofs.write((char*)&data_short[0], dataChunkSize);
    size_all += sizeof(char)*4 + sizeof(long) + dataChunkSize;
  }

  // add filesize
  ofs.seekp(4, ios::beg);
  ofs.write((char*)&(size_all), sizeof(long));
}

bool UnitWaveformContainer::isUwcFormatFile(const boost::filesystem::path& path_uwc)
{
  boost::filesystem::ifstream ifs(path_uwc, ios_base::binary);

  char tag[4];
  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavHeader::isRiffTag(tag)) {
    return false;
  }
  ifs.seekg(8, ios_base::beg);
  ifs.read((char*)&tag, sizeof(char)*4);
  if (!WavHeader::isWaveTag(tag)) {
    return false;
  }
  long chunkSize = 0;
  ifs.seekg(16, ios::beg);
  ifs.read((char*)&chunkSize, sizeof(long));
  if (chunkSize != UnitWaveformHeader::const_chunk_size) {
    return false;
  }
  unsigned short wFormatTag = 0;
  ifs.seekg(20, ios::beg);
  ifs.read((char*)&wFormatTag, sizeof(short));
  if (wFormatTag != UnitWaveformHeader::UnitWaveformFormatTag) {
    return false;
  }
  return true;
}

void UnitWaveformContainer::clear()
{
  header = UnitWaveformHeader();
  std::vector<UnitWaveform>(unit_waveforms).swap(unit_waveforms);
}
