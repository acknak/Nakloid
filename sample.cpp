#include "Nakloid.h"

int main()
{
  Nakloid *nakloid = new Nakloid("./input/input.ust");
//Nakloid *nakloid = new Nakloid("voiceDB", "./input/score.mid", 1, "./input/lyric.txt", "./output/output.wav");
//nakloid->setMargin(1000);
  nakloid->vocalization();
  delete nakloid;
  return 0;
}
