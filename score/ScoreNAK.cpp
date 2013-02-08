#include "ScoreNAK.h"

using namespace std;

ScoreNAK::ScoreNAK(string input_NAK, string path_pitches, string path_song, string path_singer)
  :Score(input_NAK, path_pitches, path_song, path_singer)
{
  load(input_NAK);
}

ScoreNAK::~ScoreNAK()
{
}

void ScoreNAK::load(string input_NAK)
{
  cout << "----- start score(NAK) loading -----" << endl;
  cout << "----- finish score(NAK) loading -----" << endl;
}

void ScoreNAK::output(string output_NAK)
{
  if (!isScoreLoaded()) {
    cerr << "score hasn't loaded" << endl;
    return;
  }
}
