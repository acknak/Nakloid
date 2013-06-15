#include "Score.h"

using namespace std;

Score::Score(string input, string path_song, string path_singer)
{
  this->path_song = path_song;
  this->path_singer = path_singer;
  boost::assign::push_back(key2notenum)("C")("C#")("D")("D#")("E")("F")("F#")("G")("G#")("A")("A#")("B");
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
    for (unsigned long i=it->getStart(); i<it->getEnd(); i++)
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
	    wstring tmp = wcs;
	    delete [] wcs;
      pt_note.put(L"pron", tmp);
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

bool Score::loadPitchesFromPit(std::string path_input_pitches)
{
  ifstream ifs;
  ifs.open(path_input_pitches.c_str(), ios::binary);
  if (ifs) {
    unsigned long pitches_size = boost::filesystem::file_size(path_input_pitches);
    pitches.assign(pitches_size/sizeof(float), 0.0);
    ifs.read((char*)&(pitches[0]), pitches_size);
    return (is_tempered=true);
  } else {
    cerr << "[Score::loadPitches] can't open pitches data" << endl;
  }
  return false;
}

bool Score::loadPitchesFromLf0(std::string path_input_pitches)
{
  ifstream ifs;
  ifs.open(path_input_pitches.c_str(), ios::binary);
  if (ifs) {
    unsigned long pitches_size = boost::filesystem::file_size(path_input_pitches);
    vector<float> tmp_pitches(pitches_size/sizeof(float), 0.0);
    pitches.assign(pitches_size*nak::pitch_frame_length/sizeof(float), 0.0);
    ifs.read((char*)&(tmp_pitches[0]), pitches_size);
    for (int i=0; i<tmp_pitches.size(); i++) {
      if (tmp_pitches[i] == -1e+10)
        tmp_pitches[i] = 0.0;
      else
        tmp_pitches[i] = exp(tmp_pitches[i]);
    }
    for (int i=0; i<pitches.size(); i++)
      pitches[i] = tmp_pitches[i/5];
    return (is_tempered=true);
  } else {
    cerr << "[Score::loadPitches] can't open pitches data" << endl;
  }
  return false;
}

void Score::savePitches(std::string path_output_pitches)
{
  ofstream ofs;
  ofs.open(path_output_pitches.c_str(), ios::binary);
  ofs.write((char*)&(pitches[0]), pitches.size()*sizeof(float));
}

bool Score::loadModifierMap(std::string path_modifier_map)
{
  ifstream ifs;
  ifs.open(path_modifier_map.c_str());
  string buf_str;
  if (ifs) {
    while (getline(ifs, buf_str)) {
      vector<string> str_vector;
      boost::algorithm::split(str_vector, buf_str, boost::is_any_of("\t"));
      try {
        short key_num = boost::lexical_cast<short>(str_vector[0].back());
        str_vector[0].erase(--str_vector[0].end());
        vector<string>::iterator pos;
        if ((pos=find(key2notenum.begin(),key2notenum.end(),str_vector[0])) == key2notenum.end())
          throw "";
        short notenum = (++key_num)*12 + (pos-key2notenum.begin());
        key2modifier[notenum] = make_pair(str_vector.at(1), str_vector.at(2));
      } catch (...) {
        cerr << "[Score::loadModifierMap] unexpected key: \"" << buf_str << "\"" <<endl;
      }
    }
    return true;
  } else {
    cerr << "[Score::loadModifierMap] can't find modifier map: \"" << path_modifier_map << "\"" << endl;
  }
  return false;
}

pair<string, string> Score::getModifier(short key)
{
  if (!key2modifier.empty() && key2modifier.count(key)>0)
    return key2modifier[key];
  return make_pair("", "");
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

Note* Score::getNextNote(Note *note)
{
  list<Note>::iterator it_tmp_note = find(notes.begin(), notes.end(), *note);
  if (notes.size()==0 || it_tmp_note==notes.end() || it_tmp_note==--notes.end())
    return 0;
  return &*(++it_tmp_note);
}

Note* Score::getPrevNote(Note *note)
{
  list<Note>::iterator it_tmp_note = find(notes.begin(), notes.end(), *note);
  if (notes.size()==0 || it_tmp_note==notes.end() || it_tmp_note==notes.begin())
    return 0;
  return &*(--it_tmp_note);
}
