#ifndef Note_h
#define Note_h

#include <cmath>
#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <boost/utility.hpp>
#include "Score.h"
#include "../Utilities.h"
class Score;

struct NoteFrame {
  NoteFrame():start(0),end(0),alias(),base_pitch(0x45),base_velocity(100),margin(0),padding(0),prec(0),ovrl(0),cons(0),is_vcv(false){};
  long start;
  long end;
  nak::VoiceAlias alias;
  unsigned char base_pitch;
  short base_velocity;
  std::list< std::pair<long,short> > velocity_points;
  std::pair<short,short> *margin;
  std::pair<short,short> *padding;
  short *prec;
  short *ovrl;
  short *cons;
  bool is_vcv;
};

class Note {
 public:
  Note(Score *score, long id);
  Note(Score *score, long id, unsigned long deltatime, unsigned short timebase, unsigned long tempo, unsigned char base_pitch, short base_velocity);
  Note(const Note& other);
  virtual ~Note();

  Note& operator=(const Note& other);
  bool operator==(const Note& other) const;
  bool operator!=(const Note& other) const;

  // accessor
  long getId() const;
  long getStart() const;
  long getPronStart() const;
  void setStart(long ms_start);
  void setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  long getEnd() const;
  long getPronEnd() const;
  void setEnd(long ms_end);
  void setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  short getFrontMargin();
  short getBackMargin();
  void setMargin(short front, short back);
  short getFrontPadding();
  short getBackPadding();
  void setPadding(short front, short back);
  const std::wstring& getPron() const;
  void setPron(const std::wstring& pron);
  const std::wstring& getPrefix() const;
  void setPrefix(const std::wstring& prefix);
  const std::wstring& getSuffix() const;
  void setSuffix(const std::wstring& suffix);
  nak::VoiceAlias getAlias() const;
  std::wstring getAliasString() const;
  void setAlias(nak::VoiceAlias voice_alias);
  void setAlias(const std::wstring& alias);
  unsigned char getBasePitch() const;
  float getBasePitchHz() const;
  void setBasePitch(unsigned char base_pitch);
  short getBaseVelocity() const;
  void setBaseVelocity(short base_velocity);
  void addVelocityPoint(long ms, short vel);
  void setVelocityPoints(const std::list< std::pair<long,short> >& velocity_points);
  std::list< std::pair<long,short> > getVelocityPoints();
  short getVelocityPointNum() const;
  std::vector<short> getVelocities();
  bool isPrec() const;
  short getPrec() const;
  void setPrec(short prec);
  bool isOvrl() const;
  short getOvrl() const;
  void setOvrl(short ovrl);
  bool isCons() const;
  short getCons() const;
  void setCons(short cons);
  bool isVCV() const;
  void isVCV(bool is_vcv);

 protected:
  Score *score;
  long id;
  NoteFrame self;
};

#endif
