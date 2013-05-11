#ifndef Note_h
#define Note_h

#include <map>
#include <cmath>
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <boost/utility.hpp>
#include "Score.h"
#include "../Utilities.h"
class Score;

typedef struct {
  long start;
  long end;
  std::string pron;
  std::string prefix;
  std::string suffix;
  unsigned char base_pitch;
  short base_velocity;
  std::list< std::pair<long,short> > velocity_points;
  short *prec;
  short *ovrl;
  bool is_vcv;
} NoteFrame;

// Value Object
class Note {
 public:
  Note(Score *score, unsigned long id);
  Note(Score *score, unsigned long id, unsigned long deltatime, unsigned short timebase, unsigned long tempo, unsigned char base_pitch, short base_velocity);
  Note(const Note& other);
  ~Note();

  Note& operator=(const Note& other);
  bool operator==(const Note& other) const;
  bool operator!=(const Note& other) const;

  // accessor
  unsigned long getId();
  long getStart();
  long getPronStart();
  void setStart(unsigned long ms_start);
  void setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  long getEnd();
  long getPronEnd();
  void setEnd(unsigned long ms_end);
  void setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  std::string getPron();
  void setPron(std::string pron);
  std::string getPrefix();
  void setPrefix(std::string prefix);
  std::string getSuffix();
  void setSuffix(std::string suffix);
  std::string getAlias();
  unsigned char getBasePitch();
  double getBasePitchHz();
  void setBasePitch(unsigned char base_pitch);
  short getBaseVelocity();
  void setBaseVelocity(short base_velocity);
  void addVelocityPoint(long ms, short vel);
  std::list< std::pair<long,short> > getVelocityPoints();
  short getVelocityPointNum();
  std::vector<short> getVelocities();
  short getFadeinTime();
  bool isPrec();
  short getPrec() const;
  void setPrec(short prec);
  bool isOvrl();
  short getOvrl() const;
  void setOvrl(short ovrl);
  bool isVCV();
  void isVCV(bool is_vcv);

 protected:
  Score *score;
  unsigned long id;
  NoteFrame self;

  void initializeNoteFrame();
};

#endif
