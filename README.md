Nakloid
======================
Nakloid: Unit-waveform-oriented Singing Voice Synthesis System

usage
------
    // main.cpp
    #include "Nakloid.h"
    int main () {
      Nakloid *nakloid = new Nakloid("voiceDB", "score.mid", 1, "lyric.txt");
      nakloid->setMargin(1000);
      nakloid->vocalization("./output/output.wav");
      delete nakloid;
      return 0;
    }

required
------
* Microsoft Visual C++ Compiler
* [FFTW](http://www.fftw.org/)
* [Boost C++ Libraries](http://www.boost.org/)
 
license
----------
Copyright &copy; 2012 acknak

Nakloid is released under the [BSD 3-Clause License](http://opensource.org/licenses/BSD-3-Clause)

see also
------
* [wiki](https://github.com/acknak/Nakloid/wiki)
* [video commentary](http://www.nicovideo.jp/watch/sm17093726)
* [samples](http://www.nicovideo.jp/mylist/32930257)