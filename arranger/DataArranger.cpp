#include "DataArranger.h"

using namespace std;

const short DataArranger::edge_back_ms = 30; //ms
const short DataArranger::edge_front_ms = 30; //ms

void DataArranger::edge_front(vector<short> *data, long sps)
{
  double edge_length = ms2pos(edge_front_ms, sps);
  if (data->size() < edge_length)
    for (int i=0; i<data->size(); i++)
      *(data->begin()+i) *= i/data->size();
  else
    for(int i=0; i<edge_length; i++)
      *(data->begin()+i) *= i/edge_length;
}

void DataArranger::edge_back(vector<short> *data, long sps)
{
  double edge_length = ms2pos(edge_front_ms, sps);
  if (data->size() < edge_length)
    for (int i=0; i<data->size(); i++)
      *(data->begin()+i) *= (data->size()-i)/data->size();
  else
    for(int i=0; i<edge_length; i++)
      *(data->end()-edge_length+i) *= (edge_length-i)/edge_length;
}

double DataArranger::ms2pos(long ms, long sps)
{
  return ms / 1000.0 * sps;
}
