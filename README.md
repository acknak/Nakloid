Nakloid
======================
Nakloid: Unit-waveform-oriented Singing Voice Synthesis System

sample code
------
    // sample.cpp
    #include "Nakloid.h"
    int main () {
      Nakloid *nakloid = new Nakloid("./input/input.ust");
    //Nakloid *nakloid = new Nakloid("voiceDB", "./input/score.mid", 1, "./input/lyric.txt", "./output/output.wav");
    //nakloid->setMargin(1000);
      nakloid->vocalization();
      delete nakloid;
      return 0;
    }

required
------
* [FFTW](http://www.fftw.org/)
* [Boost C++ Libraries](http://www.boost.org/)

license
----------
Copyright &copy; 2012 acknak

Nakloid is released under the [BSD 3-Clause License](http://opensource.org/licenses/BSD-3-Clause)

see also
------
* [wiki](https://github.com/acknak/Nakloid/wiki)
* [video commentary](http://nico.ms/sm17093726)
* [samples](http://nico.ms/mylist/32930257)