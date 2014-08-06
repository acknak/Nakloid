#include "WavData.h"

using namespace std;

WavData::WavData(){}

WavData::WavData(const vector<double>& data_vector):data(data_vector){}

WavData::WavData(const short* const data, long chunk_size)
{
  setData(data, chunk_size);
}

WavData::WavData(const WavData& other)
{
  data = other.getData();
}

WavData::~WavData(){}

/*
 * accessor
 */
vector<short> WavData::getWavData() const
{
  vector<short> tmp(data.size(), 0);
  WavParser::dbl2sht(data, tmp);
  return tmp;
}

const vector<double>& WavData::getData() const
{
  return data;
}

void WavData::setData(const short* const data, long chunk_size)
{
  if (data==0 || chunk_size==0) {
    cerr << "[WavData::setData] can't assign null data";
    return;
  }

  this->data.assign(chunk_size/sizeof(short), 0);
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

double WavData::getRMS() const
{
  double rms = 0.0;
  for (vector<double>::const_iterator it=data.begin(); it!=data.end(); ++it) {
    rms += pow((double)*it, 2) / data.size();
  }
  return sqrt(rms);
}
