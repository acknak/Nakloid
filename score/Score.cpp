#include "Score.h"

using namespace std;

vector<wstring> Score::key2notenum = boost::assign::list_of(L"C")(L"C#")(L"D")(L"D#")(L"E")(L"F")(L"F#")(L"G")(L"G#")(L"A")(L"A#")(L"B");

Score::Score(const wstring& input, const wstring& path_song, const wstring& path_singer)
{
  this->path_song = path_song;
  this->path_singer = path_singer;
  key2modifier[-1] = make_pair(L"",L"");
}

Score::~Score(){}

bool Score::load()
{
  return false;
}

bool Score::isScoreLoaded() const
{
  return !notes.empty();
}

void Score::reloadPitches()
{
  pitches.clear();
  pitches.resize(notes.back().getEnd(), 0.0);
  for (list<Note>::iterator it=notes.begin(); it!=notes.end(); ++it)
    for (size_t i=it->getStart(); i<it->getEnd(); i++)
      pitches[i] = it->getBasePitchHz();
}

void Score::saveScore(const wstring& path_nak)
{
  if (!isScoreLoaded()) {
    cerr << "score hasn't loaded" << endl;
    return;
  }
  boost::property_tree::wptree pt, pt_notes;
  for (list<Note>::iterator it_notes=notes.begin(); it_notes!=notes.end(); ++it_notes) {
    boost::property_tree::wptree pt_note, pt_vel_points;
    pt_note.put(L"id", it_notes->getId());
    pt_note.put(L"alias", it_notes->getAliasString());
    pt_note.put(L"vcv", it_notes->isVCV());
    pt_note.put(L"start", it_notes->getStart());
    pt_note.put(L"end", it_notes->getEnd());
    {
      wstringstream tmp_margin;
      tmp_margin << it_notes->getFrontMargin() << L"," << it_notes->getBackMargin();
      pt_note.put(L"margin", tmp_margin.str());
    }
    {
      wstringstream tmp_padding;
      tmp_padding << it_notes->getFrontPadding() << L"," << it_notes->getBackPadding();
      pt_note.put(L"padding", tmp_padding.str());
    }
    pt_note.put(L"prec", it_notes->getPrec());
    pt_note.put(L"ovrl", it_notes->getOvrl());
    pt_note.put(L"cons", it_notes->getCons());
    pt_note.put(L"vel", it_notes->getBaseVelocity());
    pt_note.put(L"pitch", it_notes->getBasePitch());
    {
      list< pair<long, short> > vel_points = it_notes->getVelocityPoints();
      for (list< pair<long, short> >::iterator it_vel_points=vel_points.begin(); it_vel_points!=vel_points.end(); ++it_vel_points) {
        boost::property_tree::wptree pt_vel_point;
        wstringstream tmp_vel_point;
        tmp_vel_point << it_vel_points->first << L"," << it_vel_points->second;
        pt_vel_point.put(L"", tmp_vel_point.str());
        pt_vel_points.push_back(make_pair(L"", pt_vel_point));
      }
      pt_note.add_child(L"vel_points", pt_vel_points);
    }
    pt_notes.push_back(make_pair(L"", pt_note));
  }
  pt.add_child(L"Score.notes", pt_notes);

  boost::filesystem::path fs_path_nak(path_nak);
  write_json(fs_path_nak.string(), pt);
}

bool Score::loadPitchesFromPit(const std::wstring& path_input_pitches)
{
  boost::filesystem::ifstream ifs(path_input_pitches, ios::binary);
  if (ifs) {
    long pitches_size = boost::filesystem::file_size(path_input_pitches);
    pitches.assign(pitches_size/sizeof(float), 0.0);
    ifs.read((char*)&(pitches[0]), pitches_size);
    return (is_tempered=true);
  } else {
    cerr << "[Score::loadPitches] can't open pitches data" << endl;
  }
  return false;
}

bool Score::loadPitchesFromLf0(const std::wstring& path_input_pitches)
{
  boost::filesystem::ifstream ifs(path_input_pitches, ios::binary);
  if (ifs) {
    long pitches_size = boost::filesystem::file_size(path_input_pitches);
    vector<float> tmp_pitches(pitches_size/sizeof(float), 0.0);
    pitches.assign(pitches_size*nak::pitch_frame_length/sizeof(float), 0.0);
    ifs.read((char*)&(tmp_pitches[0]), pitches_size);
    for (size_t i=0; i<tmp_pitches.size(); i++) {
      if (tmp_pitches[i] == -1e+10)
        tmp_pitches[i] = 0.0;
      else
        tmp_pitches[i] = exp(tmp_pitches[i]);
    }
    for (size_t i=0; i<pitches.size(); i++)
      pitches[i] = tmp_pitches[i/5];
    return (is_tempered=true);
  } else {
    cerr << "[Score::loadPitches] can't open pitches data" << endl;
  }
  return false;
}

void Score::savePitches(const std::wstring& path_output_pitches)
{
  boost::filesystem::ofstream ofs(path_output_pitches, ios::binary);
  ofs.write((char*)&(pitches[0]), pitches.size()*sizeof(float));
}

bool Score::loadModifierMap(const std::wstring& path_modifier_map)
{
  boost::filesystem::wifstream ifs(path_modifier_map);
  wstring buf_str;
  if (ifs) {
    while (getline(ifs, buf_str)) {
      vector<wstring> str_vector;
      boost::algorithm::split(str_vector, buf_str, boost::is_any_of("\t"));
      try {
        short key_num = boost::lexical_cast<short>(str_vector[0].back());
        str_vector[0].erase(--str_vector[0].end());
        vector<wstring>::iterator pos;
        if ((pos=find(key2notenum.begin(),key2notenum.end(),str_vector[0])) == key2notenum.end())
          throw "";
        short notenum = (++key_num)*12 + (pos-key2notenum.begin());
        key2modifier[notenum] = make_pair(str_vector.at(1), str_vector.at(2));
      } catch (...) {
        wcerr << L"[Score::loadModifierMap] unexpected key: \"" << buf_str << L"\"" <<endl;
      }
    }
    return true;
  } else {
    wcerr << L"[Score::loadModifierMap] can't find modifier map: \"" << path_modifier_map << L"\"" << endl;
  }
  return false;
}

/*
 * accessor
 */
const pair<wstring, wstring>& Score::getModifier(short key) const
{
  if (!key2modifier.empty() && key2modifier.count(key)>0)
    return key2modifier.at(key);
  return key2modifier.at(-1);
}

const vector<float>& Score::getPitches() const
{
  return pitches;
}

void Score::setPitches(const vector<float>& pitches)
{
  this->pitches = pitches;
}

const wstring& Score::getSongPath() const
{
  return path_song;
}

void Score::setSongPath(const wstring& path_song)
{
  this->path_song = path_song;
}

wstring Score::getSingerPath() const
{
  return path_singer;
}

void Score::setSingerPath(const wstring& path_singer)
{
  this->path_singer = path_singer;
}

bool Score::isTempered() const
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
