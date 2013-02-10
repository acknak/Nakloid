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
  unsigned char base_pitch;
  short base_velocity;
  std::list< std::pair<long,short> > velocities;
  short *prec;
  short *ovrl;
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
  unsigned long getStart();
  unsigned long getPronStart();
  void setStart(unsigned long ms_start);
  void setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  unsigned long getEnd();
  unsigned long getPronEnd();
  void setEnd(unsigned long ms_end);
  void setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  std::string getPron();
  void setPron(std::string pron);
  unsigned char getBasePitch();
  double getBasePitchHz();
  void setBasePitch(unsigned char base_pitch);
  short getBaseVelocity();
  void setBaseVelocity(short base_velocity);
  void addVelocityPoint(long ms, short vel);
  std::list< std::pair<long,short> > getVelocityPoints();
  short getVelocityPointNum();
  std::vector<short> getVelocities();
  short getLack();
  bool isPrec();
  short getPrec() const;
  void setPrec(short prec);
  bool isOvrl();
  short getOvrl() const;
  void setOvrl(short ovrl);

 protected:
  Score *score;
  unsigned long id;
  NoteFrame self;

  void initializeNoteFrame();
};

#endif
