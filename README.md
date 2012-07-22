Nakloid
======================
A Singing Voice Synthesis System with TD-PSOLA method

See also [video commentary](http://www.nicovideo.jp/watch/sm17093726) and [samples](http://www.nicovideo.jp/mylist/32930257)

What required at compile
------
* Microsoft Visual C++
* [FFTW](http://www.fftw.org/)
* [Boost C++ Libraries](http://www.boost.org/)
 
How to use
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

License
----------
Copyright &copy; 2012 acknak

Nakloid is released under the [BSD 3-Clause License](http://opensource.org/licenses/BSD-3-Clause)
