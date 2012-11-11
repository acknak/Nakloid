#include "Nakloid.h"

int main()
{
  Nakloid *nakloid = new Nakloid("./input/input.ust");
  nakloid->vocalization();
  delete nakloid;
  return 0;

//Nakloid *nakloid = new Nakloid("voiceDB", "./input/score.mid", 1, "./input/lyric.txt");
//nakloid->setMargin(1000);
//nakloid->vocalization("./output/output.wav");
//delete nakloid;
//return 0;
}
