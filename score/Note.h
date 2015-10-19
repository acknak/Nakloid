#ifndef Note_h
#define Note_h

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <boost/utility.hpp>
#include "Score.h"
#include "../format/PronunciationAlias.h"
class Score;

struct NoteFrame {
  NoteFrame() {
    start = end = 0;
    base_pitch = 0x45;
    base_velocity = 100;
    margin = padding = 0;
    preu = ovrl = cons = 0;
    is_vcv = false;
  }
  long start, end;
  PronunciationAlias pron_alias;
  unsigned char base_pitch;
  short base_velocity;
  std::vector< std::pair<long,short> > velocity_points;
  std::pair<short,short> *margin, *padding;
  short *preu, *ovrl, *cons;
  bool is_vcv;
};

class Note {
 public:
  static struct Parameters {
    Parameters() {
      ms_front_padding = 5;
      ms_back_padding = 35;
      auto_vowel_combining = false;
      vowel_combining_volume = 0.8;
    };
    short ms_front_padding;
    short ms_back_padding;
    bool auto_vowel_combining;
    double vowel_combining_volume;
  } params;

  Note(Score *score, long id);
  Note(Score *score, long id, long ms_start, unsigned char base_pitch, short base_velocity);
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
  long getEnd() const;
  long getPronEnd() const;
  void setEnd(long ms_end);
  short getFrontMargin() const;
  short getBackMargin() const;
  void setMargin(short front, short back);
  short getFrontPadding() const;
  short getBackPadding() const;
  void setPadding(short front, short back);
  PronunciationAlias getPronAlias() const;
  std::wstring getPronAliasString() const;
  void setPronAlias(PronunciationAlias pron_alias);
  void setPronAlias(const std::wstring& pron_alias);
  unsigned char getBasePitch() const;
  float getBasePitchHz() const;
  void setBasePitch(unsigned char base_pitch);
  short getBaseVelocity() const;
  void setBaseVelocity(short base_velocity);
  void addVelocityPoint(long ms, short vel);
  void setVelocityPoints(const std::vector< std::pair<long,short> > &velocity_points);
  const std::vector< std::pair<long,short> > &getVelocityPoints() const;
  short getVelocityPointNum() const;
  std::vector<short> getVelocities() const;
  bool isPreu() const;
  short getPreu() const;
  void setPreu(short preu);
  bool isOvrl() const;
  short getOvrl() const;
  void setOvrl(short ovrl);
  bool isCons() const;
  short getCons() const;
  void setCons(short cons);
  bool isVCV() const;
  void isVCV(bool is_vcv);

 private:
  Score *score;
  long id;
  NoteFrame self;
};

#endif
