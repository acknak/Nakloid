#include "Score.h"

using namespace std;

Score::Score(string input, string path_pitches, string path_song, string path_singer)
{
  this->path_song = path_song;
  this->path_singer = path_singer;
}

Score::~Score(){}

bool Score::load()
{
  return false;
}

bool Score::isScoreLoaded()
{
  return !notes.empty();
}

void Score::reloadPitches()
{
  pitches.clear();
  pitches.resize(notes.back().getEnd(), 0.0);
  for (list<Note>::iterator it=notes.begin(); it!=notes.end(); ++it)
    for (int i=it->getStart(); i<it->getEnd(); i++)
      pitches[i] = it->getBasePitchHz();
}

void Score::inputPitches(std::string path_input_pitches)
{
  ifstream ifs;
  ifs.open(path_input_pitches, ios::binary);
  unsigned long pitches_size = boost::filesystem::file_size(path_input_pitches);
  pitches.assign(pitches_size/sizeof(float), 0.0);
  ifs.read((char*)&(pitches[0]), pitches_size);
}

void Score::outputPitches(std::string path_output_pitches)
{
  ofstream ofs;
  ofs.open(path_output_pitches, ios::binary);
  ofs.write((char*)&(pitches[0]), pitches.size()*sizeof(float));
}

/*
 * Note mediator
 */
void Score::noteParamChanged(Note *note)
{
  if (notes.size() == 0)
    return;
  list<Note>::iterator it_tmp_note=find(notes.begin(), notes.end(), *note);
  if (it_tmp_note!=notes.begin()) {
    (boost::prior(it_tmp_note))->setLack(note->getPrec()-note->getOvrl());
    (boost::prior(it_tmp_note))->reloadVelocities();
  }

  note->reloadVelocities();
}


/*
 * accessor
 */
vector<float> Score::getPitches()
{
  return pitches;
}

void Score::setPitches(vector<float> pitches)
{
  this->pitches = pitches;
}

string Score::getSongPath()
{
  return path_song;
}

void Score::setSongPath(string path_song)
{
  this->path_song = path_song;
}

string Score::getSingerPath()
{
  return path_singer;
}

void Score::setSingerPath(string path_singer)
{
  this->path_singer = path_singer;
}
