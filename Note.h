#ifndef Note_h
#define Note_h

#include <cmath>
#include <string>

// Value Object
class Note {
 public:
  const static double edge_length;

  Note();
  Note(unsigned long start, unsigned char pitch, short velocity);
  Note(unsigned long deltatime, unsigned short timebase, unsigned long tempo, unsigned char pitch, short velocity);
  Note(const Note& other);
  ~Note();

  Note& operator=(const Note& other);
  bool operator==(const Note& other) const;
  bool operator!=(const Note& other) const;

  // accessor
  unsigned long getStart();
  void setStart(unsigned long start);
  void setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  unsigned long getEnd();
  void setEnd(unsigned long end);
  void setEnd(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  std::string getPron();
  void setPron(std::string pron);
  unsigned char getPitch();
  double getPitchHz();
  void setPitch(unsigned char pitch);
  unsigned short getVelocity();
  void setVelocity(short velocity);
  bool isPrec();
  unsigned short getPrec();
  void setPrec(unsigned short prec);
  bool isOvrl();
  unsigned short getOvrl();
  void setOvrl(unsigned short ovrl);

 private:
  unsigned long start;
  unsigned long end;
  std::string pron;
  unsigned char pitch;
  unsigned short velocity;
  bool is_prec;
  unsigned short prec;
  bool is_ovrl;
  unsigned short ovrl;

  double tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo);
};

#endif
