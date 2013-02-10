#include "Nakloid.h"

using namespace std;

Nakloid::Nakloid() : voice_db(0), score(0), margin(0)
{
  loadDefaultFormat();
}

Nakloid::Nakloid(nak::ScoreMode mode) : voice_db(0), score(0), margin(0)
{
  loadDefaultFormat();
  loadScore(mode);
}

Nakloid::~Nakloid()
{
  if (voice_db != 0) {
    delete voice_db;
    voice_db = 0;
  }
  if (score != 0) {
    delete score;
    score = 0;
  }
}

void Nakloid::loadDefaultFormat()
{
  format.chunkSize = 16;
  format.wFormatTag = 1;
  format.wChannels = 1;
  format.dwSamplesPerSec = 44100;
  format.dwAvgBytesPerSec = format.dwSamplesPerSec*2;
  format.wBlockAlign = 2;
  format.wBitsPerSamples = 16;
}

bool Nakloid::loadScore(nak::ScoreMode mode)
{
  if (score != 0) {
    delete score;
    score = 0;
  }

  switch(mode){
  case nak::score_mode_nak:
    score=new ScoreNAK(nak::path_nak, nak::path_pitches, nak::path_song, nak::path_singer); break;
  case nak::score_mode_ust:
    score=new ScoreUST(nak::path_ust, nak::path_pitches, nak::path_song, nak::path_singer); break;
  case nak::score_mode_smf:
    score=new ScoreSMF(nak::path_smf, nak::track, nak::path_lyrics, nak::path_pitches, nak::path_song, nak::path_singer); break;
  }
  if (score == 0 || !score->isScoreLoaded()) {
    cerr << "[Nakloid::loadScore] score hasn't loaded" << endl;
    return false;
  }

  voice_db = new VoiceDB(score->getSingerPath());

  return true;
}

bool Nakloid::vocalization()
{
  if (score == 0) {
    cerr << "[Nakloid::vocalization] score hasn't loaded" << endl;
    return false;
  }

  if (score->notes.empty()) {
    cerr << "[Nakloid::vocalization] notes hasn't loaded" << endl;
    return false;
  }

  if (voice_db == 0) {
    cerr << "[Nakloid::vocalization] can't find voiceDB" << endl;
    return false;
  }

  cout << "----- start vocalization -----" << endl << endl;
  setMargin(nak::margin);

  // set note params from voiceDB
  if (nak::score_mode != nak::score_mode_nak) {
    cout << endl << "loading voiceDB..." << endl << endl;
    double counter=0, percent=0;
    for (list<Note>::iterator it_notes=score->notes.begin(); it_notes!=score->notes.end(); ++it_notes) {
      // vowel combining
      if (nak::vowel_combining) {
        if (it_notes!=score->notes.begin() && boost::prior(it_notes)->getEnd()==it_notes->getStart()) {
          if (voice_db->isPron("* "+it_notes->getPron())) {
            it_notes->setPron("* "+it_notes->getPron());
            if (nak::isVowel(it_notes->getPron()))
              it_notes->setBaseVelocity(it_notes->getBaseVelocity()*nak::vowel_combining_volume);
          }
        } else {
          if (voice_db->isPron("- "+it_notes->getPron()))
            it_notes->setPron("- "+it_notes->getPron());
        }
      }

      // set overlap range & preceding utterance
      if (it_notes == score->notes.begin()) {
        short ovrl = it_notes->isOvrl()?it_notes->getOvrl():voice_db->getVoice(it_notes->getPron()).ovrl;
        short prec = it_notes->isPrec()?it_notes->getPrec():voice_db->getVoice(it_notes->getPron()).prec;
        short tmp_margin = prec - ((ovrl<0)?ovrl:0);
        if (max<unsigned long>(margin, tmp_margin) > it_notes->getStart())
          margin = max<unsigned long>(margin, tmp_margin);
      }
      if (!it_notes->isOvrl())
        it_notes->setOvrl(voice_db->getVoice(it_notes->getPron()).ovrl);
      if (!it_notes->isPrec())
        it_notes->setPrec(voice_db->getVoice(it_notes->getPron()).prec);

      // show progress
      if (++counter/score->notes.size()>percent+0.1 && (percent=floor(counter/score->notes.size()*10)/10.0)<1.0)
        cout << percent*100 << "%..." << endl;
    }
    cout << endl << "load finished" << endl << endl << endl;
  }

  // arrange note params
  cout << endl << "arrange params..." << endl << endl;
  if (nak::score_mode != nak::score_mode_nak)
    NoteArranger::arrange(score);
  if (nak::path_pitches.empty())
    PitchArranger::arrange(score);
  cout << endl << "arrange finished" << endl << endl << endl;

  // Singing Voice Synthesis
  BaseWavsOverlapper *overlapper = new BaseWavsOverlapper(format, score->getPitches());
  for (list<Note>::iterator it_notes=score->notes.begin(); it_notes!=score->notes.end(); ++it_notes) {
    cout << "pron: " << it_notes->getPron() << ", "
         << "pron start: " << it_notes->getPronStart() << ", "
         << "pron end: " << it_notes->getPronEnd() << endl;
    overlapper->overlapping(it_notes->getPronStart(), it_notes->getPronEnd(),
      voice_db->getVoice(it_notes->getPron()).bwc, it_notes->getVelocities());
  }
  overlapper->outputWav(score->getSongPath(), margin);
  delete overlapper;

  cout << "----- vocalization finished -----" << endl << endl;

  return true;
}


/*
 * accessor
 */
WavFormat Nakloid::getFormat()
{
  return format;
}

void Nakloid::setFormat(WavFormat format)
{
  this->format = format;
}

Score* Nakloid::getScore()
{
  return score;
}

void Nakloid::setMargin(long margin)
{
  this->margin = margin;
}

long Nakloid::getMargin()
{
  return this->margin;
}

/*
 * main
 */
int main()
{
  setlocale(LC_CTYPE, "");

  if (!nak::parse("Nakloid.ini"))
    return 1;

  if (!nak::path_log.empty())
    freopen(nak::path_log.c_str(), "w", stdout);

  Nakloid *nakloid = new Nakloid(nak::score_mode);
  nakloid->vocalization();

  if (!nak::path_output_nak.empty())
    nakloid->getScore()->saveScore(nak::path_output_nak);

  if (!nak::path_output_pit.empty())
    nakloid->getScore()->savePitches(nak::path_output_pit);

  delete nakloid;

  if (nak::path_log.empty()) {
    cin.sync();
    cout << endl << endl << endl << "Press Enter/Return to continue..." << endl;
    cin.get();
  }

  return 0;
}
