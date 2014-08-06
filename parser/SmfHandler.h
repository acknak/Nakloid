#ifndef SmfHandler_h
#define SmfHandler_h

#include <fstream>
#include <map>
#include <string>

enum MidiMsg{
  MIDI_MSG_NOTE_ON,
  MIDI_MSG_NOTE_OFF,
  MIDI_MSG_PLY_KEY_PRS,
  MIDI_MSG_CTL_CHG,
  MIDI_MSG_PGM_CHG,
  MIDI_MSG_CHN_PRS,
  MIDI_MSG_PIT_BND_CHG,
  MIDI_MSG_EXC,
  MIDI_MSG_QURT_FRM,
  MIDI_MSG_SNG_POS_PTR,
  MIDI_MSG_SNG_SEL,
  MIDI_MSG_TUNE_REQ,
  MIDI_MSG_END_EXL,
  MIDI_MSG_META,
  MIDI_MSG_UNKNOWN
};

class SmfHandler {
 public:
  SmfHandler();
  virtual ~SmfHandler();

  virtual void smfInfo(short numTrack, short timebase){};
  virtual void trackChange(short track){};
  virtual void eventMidi(long deltatime, unsigned char msg, const unsigned char* const data){};
  virtual void eventSysEx(long deltatime, long datasize, const unsigned char* const data){};
  virtual void eventMeta(long deltatime, unsigned char type, long datasize, const unsigned char* const data){};

  static MidiMsg charToMidiMsg(unsigned char midi_msg);
  static std::string midiMsgToString(MidiMsg midi_msg);

 private:
  SmfHandler(const SmfHandler& other);
  SmfHandler& operator=(const SmfHandler& other);
};

#endif
