#include "ScoreSMF.h"

using namespace std;

ScoreSMF::ScoreSMF(string input_smf, short track, string path_lyric, string path_pitches, string path_song, string path_singer)
  :Score(input_smf, path_pitches, path_song, path_singer)
{
  load(input_smf, track, path_lyric);
}

ScoreSMF::~ScoreSMF() {}

bool ScoreSMF::load(string input, short track, string path_lyric)
{
  cout << "----- start score(smf) loading -----" << endl;

  // load lyric txt
  list<string> prons;
  ifstream ifs(path_lyric.c_str());
  string buf_str;

  while (ifs && getline(ifs, buf_str)) {
    if (buf_str == "")
      continue;
    if (*(buf_str.end()-1) == ',')
      buf_str.erase(buf_str.end()-1,buf_str.end());
    vector<string> buf_vector;
    boost::algorithm::split(buf_vector, buf_str, boost::is_any_of(","));
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
      cerr << "[Score::loadSmf] " << input << " cannot open\n";
      return false;
    }
    Note *note = 0;
    while (!ifs.eof()) {
      ifs.read((char *)note, sizeof(Note));
      notes.push_back(*note);
    }
  }
  if (notes.size() == 0) {
    cerr << "[Score::loadSmf] cannot read notes" << endl;
    return false;
  }
  list<Note>::iterator it_notes = notes.begin();
  list<string>::iterator it_prons = prons.begin();
  for (; it_notes!=notes.end()&&it_prons!=prons.end(); ++it_notes,++it_prons)
    (*it_notes).setPron(*it_prons);

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

void ScoreSMF::eventMidi(long deltatime, unsigned char msg, unsigned char* data)
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

void ScoreSMF::eventSysEx(long deltatime, long datasize, unsigned char* data){
  if (is_parse)
    time_parse += deltatime;
}

void ScoreSMF::eventMeta(long deltatime, unsigned char type, long datasize, unsigned char* data)
{
  if (is_parse)
    time_parse += deltatime;

  if (type == 0x51 && datasize == 3) {
    tempo = data[0] << 16;
    tempo += data[1] << 8;
    tempo += data[2];
  }
}
