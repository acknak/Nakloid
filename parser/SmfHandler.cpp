#include "SmfHandler.h"

using namespace std;

SmfHandler::SmfHandler(){}

SmfHandler::~SmfHandler(){}

MidiMsg SmfHandler::charToMidiMsg(unsigned char midi_msg)
{
  switch(midi_msg>>4) {
  case 0x9: return MIDI_MSG_NOTE_ON;
  case 0x8: return MIDI_MSG_NOTE_OFF;
  case 0xA: return MIDI_MSG_PLY_KEY_PRS;
  case 0xB: return MIDI_MSG_CTL_CHG;
  case 0xC: return MIDI_MSG_PGM_CHG;
  case 0xD: return MIDI_MSG_CHN_PRS;
  case 0xE: return MIDI_MSG_PIT_BND_CHG;
  }
  switch(midi_msg){
  case 0xF0: return MIDI_MSG_EXC;
  case 0xF1: return MIDI_MSG_QURT_FRM;
  case 0xF2: return MIDI_MSG_SNG_POS_PTR;
  case 0xF3: return MIDI_MSG_SNG_SEL;
  case 0xF6: return MIDI_MSG_TUNE_REQ;
  case 0xF7: return MIDI_MSG_END_EXL;
  case 0xFF: return MIDI_MSG_META;
  }
  return MIDI_MSG_UNKNOWN;
}

string SmfHandler::midiMsgToString(MidiMsg midi_msg)
{
  switch(midi_msg){
  case MIDI_MSG_NOTE_ON: return "Note On";
  case MIDI_MSG_NOTE_OFF: return "Note Off";
  case MIDI_MSG_PLY_KEY_PRS: return "Polyphonic Key Pressure";
  case MIDI_MSG_CTL_CHG: return "Control Change";
  case MIDI_MSG_PGM_CHG: return "Program Change";
  case MIDI_MSG_CHN_PRS: return "Channel Pressure";
  case MIDI_MSG_PIT_BND_CHG: return "Pitchbend Chang";
  case MIDI_MSG_EXC: return "Exclusive Message";
  case MIDI_MSG_QURT_FRM: return "Quarter Frame";
  case MIDI_MSG_SNG_POS_PTR: return "Song Position Pointer";
  case MIDI_MSG_SNG_SEL: return "Song Select";
  case MIDI_MSG_TUNE_REQ: return "Tune Request";
  case MIDI_MSG_END_EXL: return "End of Exclusive";
  case MIDI_MSG_META: return "Meta Message";
  default:;
  }
  return "Unkenown Message";
}
