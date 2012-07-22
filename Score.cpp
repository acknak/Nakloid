#include "Score.h"

using namespace std;

Score::Score() : tempo(500000), track(1), time_parse(0), note_parse(0){}

Score::Score(string input, short track) : tempo(500000), track(1), time_parse(0), note_parse(0)
{
  setTrack(track);
  load(input);
}

Score::~Score()
{
  cout << "----- finish score loading -----" << endl;
}

list<Note> Score::getNotesList()
{
  return notes;
}

vector<Note> Score::getNotesVector()
{
  vector<Note> v_notes(notes.begin(), notes.end());
  return v_notes;
}

void Score::setNote(list<Note> notes)
{
  this->notes.clear();
  this->notes = notes;
}

void Score::setTrack(short track)
{
  this->track = track;
}

void Score::load(string input)
{
  cout << "----- start score loading -----" << endl;

  SmfParser *smf_parser = new SmfParser(input);
  if (smf_parser->isSmfFile()) {
    smf_parser->addSmfHandler(this);
    smf_parser->parse();
    is_parse = false;
  } else {
    ifstream ifs;
    ifs.open(input.c_str(), ios::in|ios::binary);
    if (!ifs) {
      cout << input << " cannot open\n";
      return;
    }
    Note *note = 0;
    while (!ifs.eof()) {
      ifs.read((char *)note, sizeof(Note));
      notes.push_back(*note);
    }
  }
}
bool Score::isScoreLoaded()
{
  return !notes.empty();
}

void Score::debug(string output)
{
  ofstream ofs;
  ofs.open(output.c_str());

  ofs << setw(8) << "timebase" << setw(8) << timebase << endl
      << setw(8) << "tempo" << setw(8) << tempo << endl << endl << endl;
  ofs << setw(8) << "start"
      << setw(8) << "length"
      << setw(6) << "pit"
      << setw(6) << "vel" << endl << endl;
  for (list<Note>::iterator it=notes.begin(); it!=notes.end(); ++it)
    ofs << setw(8) << dec << it->getStart()
        << setw(8) << it->getLength()
        << setw(6) << hex << (unsigned int)it->getPitch()
        << setw(6) << (unsigned int)it->getVelocity() << endl;
}

double Score::tickToMSec(unsigned long tick)
{
  return ((double)tick) / timebase * (tempo/1000.0);
}

/*
 * inherit from SmfParser 
 */
void Score::smfInfo(short numTrack, short timebase)
{
  this->timebase = timebase;
}

void Score::trackChange(short track)
{
  time_parse = 0;
  is_parse = (this->track == track);
}

void Score::eventMidi(long deltatime, unsigned char msg, unsigned char* data)
{
  if (!is_parse)
    return;

  time_parse += deltatime;

  if (SmfHandler::charToMidiMsg(msg) == MIDI_MSG_NOTE_ON) {
    if (note_parse) {
      if (note_parse->getPitch() == data[0]) {
        if (data[1] == 0) {
          note_parse->setLength(tickToMSec(time_parse));
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
          note_parse->setLength(tickToMSec(time_parse));
          notes.push_back(*note_parse);
          delete note_parse;
          note_parse = new Note(0, data[0], data[1]);
        }
      }
    } else {
      note_parse = new Note(tickToMSec(time_parse), data[0], data[1]);
    }
    time_parse = 0;
  } else if (SmfHandler::charToMidiMsg(msg) == MIDI_MSG_NOTE_OFF && note_parse){
    note_parse->setLength(tickToMSec(time_parse));
    notes.push_back(*note_parse);
    delete note_parse;
    note_parse = NULL;
    time_parse = 0;
  }
}

void Score::eventSysEx(long deltatime, long datasize, unsigned char* data){
  if (is_parse)
    time_parse += deltatime;
}

void Score::eventMeta(long deltatime, unsigned char type, long datasize, unsigned char* data)
{
  if (is_parse)
    time_parse += deltatime;

  if (type == 0x51 && datasize == 3) {
    tempo = data[0] << 16;
    tempo += data[1] << 8;
    tempo += data[2];
  }
}
