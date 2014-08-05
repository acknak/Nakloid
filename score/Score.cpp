#include "Score.h"

using namespace std;

struct Score::Parameters Score::params;

const vector<wstring> Score::key2notenum = boost::assign::list_of(L"C")(L"C#")(L"D")(L"D#")(L"E")(L"F")(L"F#")(L"G")(L"G#")(L"A")(L"A#")(L"B");

Score::Score(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib)
  :path_score(path_score), vocal_lib(vocal_lib), path_song(L""), ms_margin(0), key2modifier()
{
  key2modifier[-1] = make_pair(L"",L"");
}

Score::Score(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib, const boost::filesystem::path& path_song)
  :path_score(path_score), vocal_lib(vocal_lib), path_song(path_song), ms_margin(0)
{
  key2modifier[-1] = make_pair(L"",L"");
}

Score::~Score(){}

void Score::saveScore(const boost::filesystem::path& path_score)
{
  boost::property_tree::wptree pt, pt_notes;
  for (vector<Note>::const_iterator it_notes=notes.begin(); it_notes!=notes.end(); ++it_notes) {
    boost::property_tree::wptree pt_note, pt_vel_points;
    pt_note.put(L"id", it_notes->getId());
    pt_note.put(L"alias", it_notes->getPronAliasString());
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
      vector< pair<long, short> > vel_points = it_notes->getVelocityPoints();
      for (vector< pair<long, short> >::iterator it_vel_points=vel_points.begin(); it_vel_points!=vel_points.end(); ++it_vel_points) {
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

  boost::filesystem::path fs_path_nak(path_score);
  write_json(fs_path_nak.string(), pt);
}

bool Score::loadModifierMap(const boost::filesystem::path& path_modifier_map)
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
        vector<wstring>::const_iterator pos;
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

bool Score::loadPitPitches(const boost::filesystem::path& path_pitches)
{
  boost::filesystem::ifstream ifs(path_pitches, ios::binary);
  if (ifs) {
    long pitches_size = boost::filesystem::file_size(path_pitches);
    pitches.assign(pitches_size/sizeof(float), 0.0);
    ifs.read((char*)&(pitches[0]), pitches_size);
    return true;
  } else {
    wcerr << L"[Score::loadFromPit] can't open pitches data " << path_pitches << endl;
  }
  return false;
}

bool Score::loadLf0Pitches(const boost::filesystem::path& path_pitches)
{
  boost::filesystem::ifstream ifs(path_pitches, ios::binary);
  if (ifs) {
    long pitches_size = boost::filesystem::file_size(path_pitches);
    vector<float> tmp_pitches(pitches_size/sizeof(float), 0.0);
    pitches.assign(pitches_size*params.pitch_frame_length/sizeof(float), 0.0);
    ifs.read((char*)&(tmp_pitches[0]), pitches_size);
    for (size_t i=0; i<tmp_pitches.size(); i++) {
      if (tmp_pitches[i] == -1e+10) {
        tmp_pitches[i] = 0.0;
      } else {
        tmp_pitches[i] = exp(tmp_pitches[i]);
      }
    }
    for (size_t i=0; i<pitches.size(); i++) {
      pitches[i] = tmp_pitches[i/5];
    }
    return true;
  } else {
    wcerr << L"[Score::loadFromLf0] can't open pitches data " << path_pitches << endl;
  }
}

void Score::savePitches(const boost::filesystem::path& path_pitches)
{
  boost::filesystem::ofstream ofs(path_pitches, ios::binary);
  ofs.write((char*)&(pitches[0]), pitches.size()*sizeof(float));
}


/*
 * accessor
 */
const boost::filesystem::path& Score::getScorePath() const
{
  return path_score;
}

const boost::filesystem::path& Score::getSongPath() const
{
  return path_song;
}

void Score::setSongPath(const boost::filesystem::path& path_song)
{
  this->path_song = path_song;
}

long Score::getMargin() const
{
  return ms_margin;
}

void Score::setMargin(long ms_margin)
{
  this->ms_margin = ms_margin;
}

vector<Note>::const_iterator Score::getNotesBegin() const
{
  return notes.begin();
}

vector<Note>::const_iterator Score::getNotesEnd() const
{
  return notes.end();
}

const Note* Score::getNextNote(const Note *note) const
{
  if (notes.size()<2) {
    return 0;
  }
  vector<Note>::const_iterator it_tmp_note = find(notes.begin(), notes.end(), *note);
  if (it_tmp_note==notes.end() || it_tmp_note==--notes.end()) {
    return 0;
  }
  return &*(++it_tmp_note);
}

const Note* Score::getPrevNote(const Note *note) const
{
  if (notes.size()<2) {
    return 0;
  }
  vector<Note>::const_iterator it_tmp_note = find(notes.begin(), notes.end(), *note);
  if (it_tmp_note==notes.begin()) {
    return 0;
  }
  return &*(--it_tmp_note);
}

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

vector<long> Score::getPitchMarks() const
{
  vector<long> pitchmarks;
  for (vector<float>::const_iterator it=pitches.begin();it!=pitches.end();++it) {
    if (*it > 0) {
      pitchmarks.assign(1, nak::ms2pos(it-pitches.begin(), params.wav_header));
      break;
    }
  }
  long ms_tmp = 0;
  while (ms_tmp < pitches.size()) {
    if (pitches[ms_tmp] > 0) {
      pitchmarks.push_back(pitchmarks.back()+(1.0/pitches[ms_tmp]*params.wav_header.dwSamplesPerSec));
      ms_tmp = nak::pos2ms(pitchmarks.back(), params.wav_header);
    } else {
      ms_tmp++;
    }
  }
  return pitchmarks;
}


/*
 * protected
 */
void Score::addNote(Note note)
{
  notes.push_back(note);
  sanitizeNote(--notes.end());
}

void Score::deleteNote(std::vector<Note>::iterator it_notes)
{
  notes.erase(it_notes);
}

void Score::clearNotes()
{
  vector<Note>().swap(notes);
}

void Score::setPitches(const vector<float>& pitches)
{
  this->pitches = pitches;
}

void Score::reloadPitches()
{
  // reload
  pitches.clear();
  pitches.resize(notes.back().getEnd()+getMargin(), 0.0);
  for (vector<Note>::const_iterator it=notes.begin(); it!=notes.end(); ++it) {
    for (long i=it->getPronStart(); i<it->getEnd()+getMargin(); i++) {
      if (i>=0 && pitches[i]==0.0) {
        pitches[i] = it->getBasePitchHz();
      }
    }
  }

  // arrange
  for (vector<Note>::const_iterator it_notes=notes.begin(); it_notes!=notes.end(); ++it_notes) {
    vector<float>::iterator it_pitches_begin=pitches.begin()+it_notes->getStart()+getMargin(), it_pitches_end=pitches.begin()+it_notes->getEnd()+getMargin();
    if (params.vibrato) {
      double tmp_fs = (nak::cent2rate(params.pitch_vibrato)-1) * *it_pitches_begin;
      if ((it_pitches_end-it_pitches_begin) > params.ms_vibrato_offset) {
        long vibrato_length = (it_pitches_end-it_pitches_begin) - params.ms_vibrato_offset;
        for (size_t i=0; i<vibrato_length; i++) {
          *(it_pitches_begin+params.ms_vibrato_offset+i) += sin(2*M_PI*i/params.ms_vibrato_width) * tmp_fs;
        }
      }
    }
    if (params.overshoot) {
      const Note* note_prev = getPrevNote(&*it_notes);
      if (note_prev!=0 && it_notes->getStart()==note_prev->getEnd() && it_notes->getBasePitch()!=note_prev->getBasePitch()) {
        double fs_tmp = (nak::cent2rate(params.pitch_overshoot)-1) * *it_pitches_begin;
        float fs_diff = (*it_pitches_begin-note_prev->getBasePitchHz()) / 2;
        if (it_pitches_end-it_pitches_begin > params.ms_overshoot) {
          for (size_t i=0; i<params.ms_overshoot/2; i++) {
            *(it_pitches_begin+i) +=
              -fs_diff + ((fs_diff+(fs_tmp*((fs_diff>0)?1:-1))) / (params.ms_overshoot/2) * i);
            *(it_pitches_begin+(params.ms_overshoot/2)+i) +=
              (fs_tmp*((fs_diff>0)?1:-1)) + ((fs_tmp*((fs_diff>0)?-1:1))/(params.ms_overshoot/2) * i);
          }
        } else {
          for (size_t i=0; i<it_pitches_end-it_pitches_begin; i++) {
            *(it_pitches_begin+i) += -fs_diff + (fs_diff/(it_pitches_end-it_pitches_begin)*i);
          }
        }
      }
    }
    if (params.preparation) {
      const Note* note_next = getNextNote(&*it_notes);
      if (note_next!=0 && it_notes->getEnd()==note_next->getStart() && it_notes->getBasePitch()!=note_next->getBasePitch()) {
        double fs_tmp = (nak::cent2rate(params.pitch_preparation)-1) * *it_pitches_begin;
        vector<float>::reverse_iterator rit_pitches_begin(it_pitches_end);
        vector<float>::reverse_iterator rit_pitches_end(it_pitches_begin);
        float fs_diff = (*rit_pitches_begin-note_next->getBasePitchHz()) / 2;
        if (rit_pitches_end-rit_pitches_begin > params.ms_preparation) {
          for (size_t i=0; i<params.ms_preparation/2; i++) {
            *(rit_pitches_begin+i) +=
              -fs_diff + ((fs_diff+(fs_tmp*((fs_diff>0)?1:-1))) / (params.ms_preparation/2) * i);
            *(rit_pitches_begin+(params.ms_preparation/2)+i) +=
              (fs_tmp*((fs_diff>0)?1:-1)) + ((fs_tmp*((fs_diff>0)?-1:1))/(params.ms_preparation/2) * i);
          }
        } else {
          for (size_t i=0; i<rit_pitches_end-rit_pitches_begin; i++) {
            *(rit_pitches_begin+i) += -fs_diff + (fs_diff/(rit_pitches_end-rit_pitches_begin)*i);
          }
        }
      }
    }
    if (params.finefluctuation) {
      boost::minstd_rand gen;
      boost::normal_distribution<> dst( 0.0, params.finefluctuation_deviation );
      boost::variate_generator<boost::minstd_rand&, boost::normal_distribution<>> rand( gen, dst );
      for (vector<float>::iterator it_pitch=it_pitches_begin; it_pitch!=it_pitches_end; ++it_pitch) {
        if (*it_pitch > 0) {
          *it_pitch += rand();
        }
      }
    }
  }
}

void Score::clearPitches()
{
  vector<float>().swap(pitches);
}


/*
 * private
 */
void Score::sanitizeNote(std::vector<Note>::iterator it_notes)
{
  PronunciationAlias tmp_alias = it_notes->getPronAlias();

  // add prefix & suffix
  if (!key2modifier.empty() && key2modifier.count(it_notes->getBasePitch())>0) {
    pair<wstring, wstring> tmp_modifier = key2modifier.at(it_notes->getBasePitch());
    tmp_alias.prefix = tmp_modifier.first + tmp_alias.prefix;
    tmp_alias.suffix = tmp_alias.suffix + tmp_modifier.second;
  }

  // vowel combining
  if (params.auto_vowel_combining && it_notes!=notes.begin()
    && boost::prior(it_notes)->getEnd()==it_notes->getStart()
    && (tmp_alias.prefix==L"*"||tmp_alias.prefix.empty())
    && vocal_lib->isAlias(L"* "+tmp_alias.pron)) {
    tmp_alias.prefix = L"* ";
  }

  // wo to o
  if (tmp_alias.pron ==L"を" && !vocal_lib->isAlias(tmp_alias.getAliasString()) && vocal_lib->isAlias(tmp_alias.prefix+L"お"+tmp_alias.suffix)) {
    tmp_alias.pron = L"お";
  }

  // complement parameters
  const Voice *tmp_voice = vocal_lib->getVoice(tmp_alias.getAliasString());
  if (tmp_voice != 0) {
    it_notes->setOvrl((it_notes->isOvrl())?it_notes->getOvrl():tmp_voice->ovrl);
    it_notes->setPrec((it_notes->isPrec())?it_notes->getPrec():tmp_voice->prec);
    it_notes->setCons((it_notes->isCons())?it_notes->getCons():tmp_voice->cons);
  }

  it_notes->setPronAlias(tmp_alias);
}
