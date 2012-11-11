#ifndef Note_h
#define Note_h

#include <cmath>
#include <string>

// Value Object
class Note {
 public:
  const static double edge_length;

  Note();
  Note(double start, unsigned char pitch, short velocity);
  Note(const Note& other);
  ~Note();

  Note& operator=(const Note& other);
  bool operator==(const Note& other) const;
  bool operator!=(const Note& other) const;

  // accessor
  double getStart();
  void setStart(double start);
  void setStart(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  double getLength();
  void setLength(double length);
  void setLength(unsigned long deltatime, unsigned short timebase, unsigned long tempo);
  std::string getPron();
  void setPron(std::string pron);
  unsigned char getPitch();
  double getPitchHz();
  void setPitch(unsigned char pitch);
  unsigned short getVelocity();
  void setVelocity(short velocity);
  short getPrec();
  void setPrec(short prec);
  short getOvrl();
  void setOvrl(short ovrl);

 private:
  double start;
  double length;
  std::string pron;
  unsigned char pitch;
  unsigned short velocity;
  short prec;
  short ovrl;

  double tick2ms(unsigned long tick, unsigned short timebase, unsigned long tempo);
};

#endif
