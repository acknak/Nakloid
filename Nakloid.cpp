#include "Nakloid.h"

using namespace std;

Nakloid::Nakloid() : voice_db(0), score(0)
{
  format = nak::getDefaultFormat();
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

bool Nakloid::vocalization()
{
  cout << "----- load voice DB -----" << endl;
  if (voice_db != 0) {
    delete voice_db;
    voice_db = 0;
  }
  voice_db = new VoiceDB(nak::path_singer);
  if (voice_db==0 || !voice_db->initVoiceMap()) {
    cerr << "[Nakloid::vocalization] can't find voiceDB" << endl;
    return false;
  }

  // load score
  cout << endl << "----- load score -----" << endl;
  switch(nak::score_mode){
  case nak::score_mode_nak:
    score=new ScoreNAK(nak::path_nak, voice_db, nak::path_song); break;
  case nak::score_mode_ust:
    score=new ScoreUST(nak::path_ust, voice_db, nak::path_song); break;
  case nak::score_mode_smf:
    score=new ScoreSMF(nak::path_smf, voice_db, nak::path_song, nak::track, nak::path_lyrics); break;
  }
  if (!nak::path_prefix_map.empty()) {
    score->loadModifierMap(nak::path_prefix_map);
    cout << "use modifier map..." << endl;
  }
  score->setMargin(nak::margin);
  score->load();
  score->loadPitches(nak::path_pitches, nak::pitches_mode);
  if (score->getNotesBegin() == score->getNotesEnd()) {
    cerr << "[Nakloid::vocalization] can't load notes" << endl;
    return false;
  }
  if (score->getPitches().size() == 0) {
    cerr << "[Nakloid::vocalization] can't load pitches" << endl;
    return false;
  }
  
  cout << endl << "----- start vocalization -----" << endl;

  // synthesize singing voice 
  UnitWaveformOverlapper *overlapper = new UnitWaveformOverlapper(format, score->getPitchMarks(format));
  double counter=0, percent=0;
  long notes_size = score->getNotesEnd() - score->getNotesEnd();
  for (vector<Note>::const_iterator it_notes=score->getNotesBegin(); it_notes!=score->getNotesEnd(); ++it_notes) {
    wcout << L"synthesize \"" << it_notes->getAliasString() << L"\" from " << it_notes->getPronStart() << L"ms to " << it_notes->getPronEnd() << L"ms" << endl;
    /*
    cout << "ovrl: " << it_notes->getOvrl() << ", prec: " << it_notes->getPrec() << ", cons: " << it_notes->getCons() << endl
      << "start: " << it_notes->getStart() << ", end: " << it_notes->getEnd() << endl
      << "front margin: "  << it_notes->getPronStart()+it_notes->getFrontMargin()
      << ", front padding: " << it_notes->getPronStart()+it_notes->getFrontMargin()+it_notes->getFrontPadding() << endl
      << "back padding: " << it_notes->getPronEnd()-it_notes->getBackPadding()-it_notes->getBackMargin()
      << ", back margin: " << it_notes->getPronEnd()-it_notes->getBackMargin() << endl;
    */
    if (voice_db->getVoice(it_notes->getAliasString()) == 0) {
      continue;
    }
    overlapper->overlapping(voice_db->getVoice(it_notes->getAliasString())->getUwc(), make_pair(it_notes->getPronStart(), it_notes->getPronEnd()), it_notes->getFrontMargin(), it_notes->getVelocities());

    // show progress
    if (++counter/notes_size>percent+0.1 && (percent=floor(counter/notes_size*10)/10.0)<1.0)
      cout << endl << percent*100 << "%..." << endl << endl;
  }
  cout << endl;
  if (nak::wav_normalize) {
    overlapper->outputNormalization();
  }
  if (nak::compressor) {
    overlapper->outputCompressing();
  }
  overlapper->outputWav(score->getSongPath());
  delete overlapper;

  cout << "----- vocalization finished -----" << endl << endl;

  return true;
}


/*
 * accessor
 */
const WavFormat& Nakloid::getFormat() const
{
  return format;
}

void Nakloid::setFormat(const WavFormat& format)
{
  this->format = format;
}

Score* Nakloid::getScore() const
{
  return score;
}

/*
 * main
 */
int main()
{
  // set locale
	ios_base::sync_with_stdio(false);
	locale default_loc("");
	locale::global(default_loc);
	locale ctype_default(locale::classic(), default_loc, locale::ctype);
	wcout.imbue(ctype_default);
	wcerr.imbue(ctype_default);
	wcin.imbue(ctype_default);

  if (!nak::parse(L"Nakloid.ini")) {
    cin.sync();
    cout << "can't open Nakloid.ini" << endl;
    cin.get();
    return 1;
  }

  if (!nak::print_log) {
    freopen("", "r", stdout);
    freopen("", "r", stderr);
  }

  Nakloid *nakloid = new Nakloid();
  nakloid->vocalization();

  if (!nak::path_output_nak.empty())
    nakloid->getScore()->saveScore(nak::path_output_nak);

  if (!nak::path_output_pit.empty())
    nakloid->getScore()->savePitches(nak::path_output_pit);

  delete nakloid;

  if (nak::print_log) {
    cin.sync();
    cout << "Press Enter/Return to continue..." << endl;
    cin.get();
  }

  return 0;
}
