#include "ScoreNML.h"

using namespace std;

ScoreNML::ScoreNML(string input_nml, string path_pitches, string path_song, string path_singer)
  :Score(input_nml, path_pitches, path_song, path_singer)
{
  load(input_nml);
}

ScoreNML::~ScoreNML()
{
}

void ScoreNML::load(string input_nml)
{
  cout << "----- start score(nml) loading -----" << endl;
  cout << "----- finish score(nml) loading -----" << endl;
}
