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
  data = other.getData();
}

WavData::~WavData(){}

WavData& WavData::operator=(const WavData& other)
{
  if (this != &other) {
    data = other.getData();
  }
  return *this;
}

bool WavData::operator==(const WavData& other) const
{
  return data==other.getData();
}

bool WavData::operator!=(const WavData& other) const
{
  return !(*this == other);
}

vector<short> WavData::getWavData() const
{
  vector<short> tmp(data.size(), 0);
  WavParser::dbl2sht(&data, &tmp);
  return tmp;
}

vector<double> WavData::getData() const
{
  return data;
}

vector<double>::const_iterator WavData::getDataIterator() const
{
  return data.begin();
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

long WavData::getSize() const
{
  return data.size();
}
