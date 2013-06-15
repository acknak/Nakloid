#include "WavData.h"

using namespace std;

WavData::WavData(){}

WavData::WavData(vector<double> data_vector)
{
  setData(data_vector);
}

WavData::WavData(const short* data, long size)
{
  setData(data, size);
}

WavData::WavData(const WavData& other)
{
  data = other.getDataVector();
}

WavData::~WavData(){}

WavData& WavData::operator=(const WavData& other)
{
  if (this != &other) {
    data = other.getDataVector();
  }
  return *this;
}

bool WavData::operator==(const WavData& other) const
{
  return data==other.getDataVector();
}

bool WavData::operator!=(const WavData& other) const
{
  return !(*this == other);
}

const short* WavData::getData() const
{
  if (data.size() == 0) {
    cerr << "[WavData::getData] can't find wav data" << endl;
    return 0;
  }

  vector<short> data_short(getWavDataSize()/sizeof(short), 0);
  for (int i=0; i<data.size(); i++) {
    data_short[i] = data[i]*32767;
  }
  return &data_short[0];
}

vector<double> WavData::getDataVector() const
{
  return data;
}

void WavData::setData(const short* data, long size)
{
  if (data==0 || size==0) {
    cerr << "[WavData::setData] can't assign null data";
    return;
  }

  this->data.assign(size/sizeof(short), 0);
  for (int i=0; i<this->data.size(); i++) {
    this->data[i] = data[i]/32768.0;
  }
}

void WavData::setData(vector<double> data)
{
  this->data = data;
}

long WavData::getWavDataSize() const
{
  return data.size()/sizeof(short);
}
