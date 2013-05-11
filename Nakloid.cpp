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

  // load score
  switch(mode){
  case nak::score_mode_nak:
    score=new ScoreNAK(nak::path_nak, nak::path_song, nak::path_singer); break;
  case nak::score_mode_ust:
    score=new ScoreUST(nak::path_ust, nak::path_song, nak::path_singer); break;
  case nak::score_mode_smf:
    score=new ScoreSMF(nak::path_smf, nak::track, nak::path_lyrics, nak::path_song, nak::path_singer); break;
  }
  if (score == 0 || !score->isScoreLoaded()) {
    cerr << "[Nakloid::loadScore] score hasn't loaded" << endl;
    return false;
  }
  cout << endl;

  // load pitches
  if (nak::pitches_mode == nak::pitches_mode_pit)
    score->loadPitchesFromPit(nak::path_pitches);
  else if (nak::pitches_mode == nak::pitches_mode_lf0)
    score->loadPitchesFromLf0(nak::path_pitches);
  else
    score->reloadPitches();

  // load prefix map
  if (nak::path_prefix_map != "") {
    score->loadModifierMap(nak::path_prefix_map);
    cout << "use modifier map..." << endl;
  }

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

  if (voice_db != 0) {
    delete voice_db;
    voice_db = 0;
  }
  voice_db = new VoiceDB(score->getSingerPath());
  if (voice_db==0 || !voice_db->initVoiceMap()) {
    cerr << "[Nakloid::vocalization] can't find voiceDB" << endl;
    return false;
  }
  cout << endl;

  cout << "----- start vocalization -----" << endl;
  setMargin(nak::margin);

  // set notes & arrange pitches
  Arranger::arrange(voice_db, score);

  // Singing Voice Synthesis
  UnitWaveformOverlapper *overlapper = new UnitWaveformOverlapper(format, score->getPitches());
  double counter=0, percent=0;
  for (list<Note>::iterator it_notes=score->notes.begin(); it_notes!=score->notes.end(); ++it_notes) {
    cout << "synthesize \"" << it_notes->getAlias() << "\" from " << it_notes->getPronStart() << "ms to " << it_notes->getPronEnd() << "ms" << endl;

    overlapper->overlapping(voice_db->getVoice(it_notes->getAlias())->getUwc(), it_notes->getPronStart(), it_notes->getPronEnd(), it_notes->getVelocities());

    // show progress
    if (++counter/score->notes.size()>percent+0.1 && (percent=floor(counter/score->notes.size()*10)/10.0)<1.0)
      cout << endl << percent*100 << "%..." << endl << endl;
  }
  cout << endl;
  overlapper->outputWav(score->getSongPath(), margin);
  delete overlapper;

  cout << "----- vocalization finished -----" << endl;
  cout << endl;

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

  if (!nak::parse("Nakloid.ini")) {
    cin.sync();
    cout << "can't open Nakloid.ini" << endl;
    cin.get();
    return 1;
  }

  if (!nak::path_log.empty()) {
    freopen(nak::path_log.c_str(), "w", stdout);
    freopen(nak::path_log.c_str(), "w", stderr);
  }

  Nakloid *nakloid = new Nakloid(nak::score_mode);
  nakloid->vocalization();

  if (!nak::path_output_nak.empty())
    nakloid->getScore()->saveScore(nak::path_output_nak);

  if (!nak::path_output_pit.empty())
    nakloid->getScore()->savePitches(nak::path_output_pit);

  delete nakloid;

  if (nak::path_log.empty()) {
    cin.sync();
    cout << "Press Enter/Return to continue..." << endl;
    cin.get();
  }

  return 0;
}
