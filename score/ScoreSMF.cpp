#include "ScoreSMF.h"

using namespace std;

ScoreSMF::ScoreSMF(const wstring& input_smf, short track, const wstring& path_lyrics, const wstring& path_song, const wstring& path_singer)
  :Score(input_smf, path_song, path_singer),timebase(0),tempo(0),track(0),is_parse(false),time_parse(0),id_parse(0)
{
  load(input_smf, track, path_lyrics);
}

ScoreSMF::~ScoreSMF() {}

bool ScoreSMF::load(const wstring& input, short track, const wstring& path_lyrics)
{
  cout << "----- start score(smf) loading -----" << endl;

  // load lyrics txt
  list<wstring> prons;
  boost::filesystem::wifstream ifs(path_lyrics);
  wstring buf_str;

  if (!ifs) {
    cerr << "[ScoreSMF::load] can't load lyrics file" << endl;
    return false;
  }
  while (getline(ifs, buf_str)) {
    if (buf_str.empty())
      continue;
    if (*(buf_str.end()-1) == L',')
      buf_str.erase(buf_str.end()-1,buf_str.end());
    vector<wstring> buf_vector;
    boost::algorithm::split(buf_vector, buf_str, boost::is_any_of(L","));
    prons.insert(prons.end(), buf_vector.begin(), buf_vector.end());
  }

  // load smf
  this->track = track;
  SmfParser *smf_parser = new SmfParser(input);
  if (smf_parser->isSmfFile()) {
    smf_parser->addSmfHandler(this);
    smf_parser->parse();
    is_parse = false;
  } else {
    ifstream ifs;
    ifs.open(input.c_str(), ios::in|ios::binary);
    if (!ifs) {
      wcerr << L"[Score::loadSmf] " << input << L" cannot open\n";
      return false;
    }
    Note *note = 0;
    while (!ifs.eof()) {
      ifs.read((char *)note, sizeof(Note));
      notes.push_back(*note);
    }
  }

  if (notes.size() == 0) {
    wcerr << L"[Score::loadSmf] cannot read notes" << endl;
    return false;
  }

  // assign notes to pron
  list<Note>::iterator it_notes = notes.begin();
  list<wstring>::iterator it_prons = prons.begin();
  for (; it_notes!=notes.end()&&it_prons!=prons.end(); ++it_notes,++it_prons) {
    string::size_type pos_prefix = it_prons->find(L" ");
    if (pos_prefix != string::npos) {
      it_notes->setPrefix(it_prons->substr(0, pos_prefix+1));
      it_notes->setPron(it_prons->substr(pos_prefix+1));
    } else {
      it_notes->setPron(*it_prons);
    }
  }

  if (!is_tempered)
    reloadPitches();

  cout << "----- finish score(smf) loading -----" << endl;

  return true;
}


/*
 * inherit from SmfParser 
 */
void ScoreSMF::smfInfo(short numTrack, short timebase)
{
  this->timebase = timebase;
}

void ScoreSMF::trackChange(short track)
{
  time_parse = 0;
  is_parse = (this->track == track);
}

void ScoreSMF::eventMidi(long deltatime, unsigned char msg, const unsigned char* const data)
{
  if (!is_parse)
    return;

  time_parse += deltatime;

  if (SmfHandler::charToMidiMsg(msg) == MIDI_MSG_NOTE_ON) {
    if (note_parse) {
      if (note_parse->getBasePitch() == data[0]) {
        if (data[1] == 0) {
          note_parse->setEnd(time_parse, timebase, tempo);
          notes.push_back(*note_parse);
          delete note_parse;
          note_parse = NULL;
        } else {
          return;
        }
      } else {
        if (data[1] == 0) {
          return;
        } else {
          note_parse->setEnd(time_parse, timebase, tempo);
          notes.push_back(*note_parse);
          delete note_parse;
          note_parse = new Note(this, ++id_parse, 0, timebase, tempo, data[0], data[1]);
        }
      }
    } else {
      note_parse = new Note(this, ++id_parse, time_parse, timebase, tempo, data[0], data[1]);
    }
  } else if (SmfHandler::charToMidiMsg(msg) == MIDI_MSG_NOTE_OFF && note_parse){
    note_parse->setEnd(time_parse, timebase, tempo);
    notes.push_back(*note_parse);
    delete note_parse;
    note_parse = NULL;
  }
}

void ScoreSMF::eventSysEx(long deltatime, long datasize, const unsigned char* const data){
  if (is_parse)
    time_parse += deltatime;
}

void ScoreSMF::eventMeta(long deltatime, unsigned char type, long datasize, const unsigned char* const data)
{
  if (is_parse)
    time_parse += deltatime;

  if (type == 0x51 && datasize == 3) {
    tempo = data[0] << 16;
    tempo += data[1] << 8;
    tempo += data[2];
  }
}
