#include "WavData.h"

using namespace std;

WavData::WavData() : data(0) {}

WavData::WavData(vector<short> data_vector) : data(0)
{
  setData(data_vector);
}

WavData::WavData(const short* data, long size) : data(0)
{
  if (data != 0) {
    setSize(size);
    this->data = new short[this->size/sizeof(short)];
    memcpy(this->data, data, this->size);
  }
}

WavData::WavData(const WavData& other) : data(0)
{
  if (other.data != 0) {
    size = other.size;
    data = new short[size/sizeof(short)];
    memcpy(data, other.data, size);
  }
}

WavData::~WavData()
{
  if (data != 0)
    delete[] data;
  data = 0;
}

WavData& WavData::operator=(const WavData& other)
{
  if (this != &other) {
    size = other.size;
    setData(other.data, other.size);
  }
  return *this;
}

bool WavData::operator==(const WavData& other) const
{
  if (size != other.size) {
    return false;
  }
  if (data == other.data) {
    return true;
  }
  if (data == 0 || other.data == 0) {
    return false;
  }
  return memcmp(data, other.data, size) == 0;
}

bool WavData::operator!=(const WavData& other) const
{
  return !(*this == other);
}

const short* WavData::getData() const
{
  return data;
}

list<short> WavData::getDataList() const
{
  list<short> data_list(data, data+(size/sizeof(short)));

  return data_list;
}

vector<short> WavData::getDataVector() const
{
  vector<short> data_vector(data, data+(size/sizeof(short)));

  return data_vector;
}

void WavData::setData(const short* data, long size)
{
  if (this->data == data) return;

  delete[] this->data;
  this->data = 0;

  if (data != 0) {
    setSize(size);
    this->data = new short[size/sizeof(short)];
    memcpy(this->data, data, size);
  }
}

void WavData::setData(list<short> data)
{
  short *data_array = new short[data.size()];
  list<short>::iterator it = data.begin();
  for (int i=0; it!=data.end(); i++,++it)
    data_array[i] = *it;
  setData(data_array, data.size()*sizeof(short));
  delete[] data_array;
}

void WavData::setData(vector<short> data)
{
  short *data_array = new short[data.size()];
  for (int i=0; i<data.size(); i++)
    data_array[i] = data[i];

  setData(data_array, data.size()*sizeof(short));
  delete[] data_array;
}

long WavData::getSize()
{
  return size;
}

void WavData::setSize(long size)
{
  this->size = size - (size % sizeof(short));
}
