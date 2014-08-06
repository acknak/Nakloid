#include "ScoreSMF.h"

using namespace std;

void ScoreSMF::load()
{
  timebase = tempo = time_parse = id_parse = 0;
  is_parse = false;
  clearNotes();

  wcout << L"smf: " << getScorePath() << endl;

  // load lyrics txt
  boost::filesystem::wifstream ifs(path_lyrics);
  wstring buf_str;
  if (!ifs) {
    cerr << "[ScoreSMF::load] can't load lyrics file" << endl;
    return;
  }
  while (getline(ifs, buf_str)) {
    if (buf_str.empty()) {
      continue;
    } if (*(buf_str.end()-1) == L',') {
      buf_str.erase(buf_str.end()-1,buf_str.end());
    }
    vector<wstring> buf_vector;
    boost::algorithm::split(buf_vector, buf_str, boost::is_any_of(L","));
    lyrics.insert(lyrics.end(), buf_vector.begin(), buf_vector.end());
  }

  // load smf
  SmfParser *smf_parser = new SmfParser(getScorePath(), this);
  if (smf_parser->isSmfFile()) {
    smf_parser->parse();
    is_parse = false;
  } else {
    wcerr << L"[ScoreSMF::load] " << getScorePath() << L" cannot open\n";
    return;
  }

  reloadPitches();

  return;
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
  is_parse = (params.smf_track == track);
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
          note_parse->setEnd(tick2ms(time_parse, timebase, tempo));
          addNote(*note_parse);
          delete note_parse;
          note_parse = 0;
        } else {
          return;
        }
      } else {
        if (data[1] == 0) {
          return;
        } else {
          note_parse->setEnd(tick2ms(time_parse, timebase, tempo));
          addNote(*note_parse);
          delete note_parse;
          note_parse = 0;
          note_parse = new Note(this, ++id_parse, 0, data[0], data[1]);
          if (id_parse <= lyrics.size()) {
            note_parse->setPronAlias(lyrics[id_parse-1]);
            if (note_parse->getPronAlias().checkVCV()) {
              note_parse->isVCV(true);
            }
          }
        }
      }
    } else {
      note_parse = new Note(this, ++id_parse, tick2ms(time_parse, timebase, tempo), data[0], data[1]);
      if (id_parse <= lyrics.size()) {
        note_parse->setPronAlias(lyrics[id_parse-1]);
        if (note_parse->getPronAlias().checkVCV()) {
          note_parse->isVCV(true);
        }
      }
    }
  } else if (SmfHandler::charToMidiMsg(msg) == MIDI_MSG_NOTE_OFF && note_parse){
    note_parse->setEnd(tick2ms(time_parse, timebase, tempo));
    addNote(*note_parse);
    delete note_parse;
    note_parse = 0;
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
