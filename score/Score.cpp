#include "Score.h"

using namespace std;

Score::Score(string input, string path_pitches, string path_song, string path_singer)
{
  this->path_song = path_song;
  this->path_singer = path_singer;
  if ((is_tempered=!path_pitches.empty())) {
    loadPitches(path_pitches);
  }
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

void Score::saveScore(string path_nak)
{
  if (!isScoreLoaded()) {
    cerr << "score hasn't loaded" << endl;
    return;
  }
  boost::property_tree::wptree pt, pt_notes;
  for (list<Note>::iterator it_notes=notes.begin(); it_notes!=notes.end(); ++it_notes) {
    boost::property_tree::wptree pt_note, pt_vel_points;
    pt_note.put(L"id", it_notes->getId());
    {
      wchar_t *wcs = new wchar_t[it_notes->getPron().length() + 1];
	    mbstowcs(wcs, it_notes->getPron().c_str(), it_notes->getPron().length() + 1);
	    wstring test = wcs;
	    delete [] wcs;
      pt_note.put(L"pron", test);
    }
    pt_note.put(L"start", it_notes->getStart());
    pt_note.put(L"end", it_notes->getEnd());
    pt_note.put(L"prec", it_notes->getPrec());
    pt_note.put(L"ovrl", it_notes->getOvrl());
    pt_note.put(L"vel", it_notes->getBaseVelocity());
    pt_note.put(L"pitch", it_notes->getBasePitch());
    list< pair<long, short> > vel_points = it_notes->getVelocityPoints();
    for (list< pair<long, short> >::iterator it_vel_points=vel_points.begin(); it_vel_points!=vel_points.end(); ++it_vel_points) {
      boost::property_tree::wptree pt_vel_point;
      wstringstream tmp_vel_point;
      tmp_vel_point << it_vel_points->first << L"," << it_vel_points->second;
      pt_vel_point.put(L"", tmp_vel_point.str());
      pt_vel_points.push_back(make_pair(L"", pt_vel_point));
    }
    pt_note.add_child(L"vel_points", pt_vel_points);
    pt_notes.push_back(make_pair(L"", pt_note));
  }
  pt.add_child(L"Score.notes", pt_notes);
  write_json(path_nak, pt);
}

void Score::loadPitches(std::string path_input_pitches)
{
  ifstream ifs;
  ifs.open(path_input_pitches.c_str(), ios::binary);
  unsigned long pitches_size = boost::filesystem::file_size(path_input_pitches);
  pitches.assign(pitches_size/sizeof(float), 0.0);
  ifs.read((char*)&(pitches[0]), pitches_size);
}

void Score::savePitches(std::string path_output_pitches)
{
  ofstream ofs;
  ofs.open(path_output_pitches.c_str(), ios::binary);
  ofs.write((char*)&(pitches[0]), pitches.size()*sizeof(float));
}

/*
 * Note mediator
 */
short Score::getNoteLack(Note *note)
{
  list<Note>::iterator it_tmp_note=find(notes.begin(), notes.end(), *note);
  if (notes.size()==0 || it_tmp_note==notes.end() || it_tmp_note==--notes.end())
    return 0;
  return boost::next(it_tmp_note)->getPrec() - boost::next(it_tmp_note)->getOvrl();
}

long Score::getNoteNextDist(Note *note)
{
  list<Note>::iterator it_tmp_note=find(notes.begin(), notes.end(), *note);
  if (notes.size()==0 || it_tmp_note==notes.end() || it_tmp_note==--notes.end())
    return 0;
  return boost::next(it_tmp_note)->getStart() - note->getEnd();
}

long Score::getNotePrevDist(Note *note)
{
  list<Note>::iterator it_tmp_note=find(notes.begin(), notes.end(), *note);
  if (notes.size()==0 || it_tmp_note==notes.end() || it_tmp_note==notes.begin())
    return 0;
  return note->getStart() - boost::prior(it_tmp_note)->getEnd();
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

bool Score::isTempered()
{
  return is_tempered;
}
