#include "Nakloid.h"

using namespace std;
struct Nakloid::Parameters Nakloid::params;

Nakloid::~Nakloid()
{
  if (vocal_lib != 0) {
    delete vocal_lib;
    vocal_lib = 0;
  }
  if (score != 0) {
    delete score;
    score = 0;
  }
}

bool Nakloid::vocalization()
{
  // load vocal library
  cout << endl << "----- load vocal library -----" << endl;
  if (vocal_lib != 0) {
    delete vocal_lib;
    vocal_lib = 0;
  }
  vocal_lib = new VocalLibrary(params.path_singer);
  vocal_lib->initVoiceMap();

  // load score
  cout << endl << "----- load score -----" << endl;
  switch(params.score_mode){
  case score_mode_nak:
    score=new ScoreNAK(params.path_input_score, vocal_lib, params.path_song); break;
  case score_mode_ust:
    score=new ScoreUST(params.path_input_score, vocal_lib, params.path_song); break;
  case score_mode_smf:
    score=new ScoreSMF(params.path_input_score, vocal_lib, params.path_song, params.path_lyrics); break;
  }
  if (!params.path_prefix_map.empty()) {
    score->loadModifierMap(params.path_prefix_map);
    cout << "use modifier map..." << endl;
  }
  score->load();
  switch(params.pitch_mode){
  case pitches_mode_pit:
    score->loadPitPitches(params.path_input_pitches); break;
  case pitches_mode_lf0:
    score->loadLf0Pitches(params.path_input_pitches); break;
  }
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
  UnitWaveformOverlapper *overlapper = new UnitWaveformOverlapper(score->getPitchMarks());
  double counter=0, percent=0;
  long notes_size = score->getNotesEnd() - score->getNotesEnd();
  for (vector<Note>::const_iterator it_notes=score->getNotesBegin(); it_notes!=score->getNotesEnd(); ++it_notes) {
    if (params.print_debug) {
      cout << endl;
    }
    wcout << L"synthesize \"" << it_notes->getPronAliasString() << L"\" from " << it_notes->getPronStart() << L"ms to " << it_notes->getPronEnd() << L"ms" << endl;
    if (params.print_debug) {
      cout << "ovrl: " << it_notes->getOvrl() << ", prec: " << it_notes->getPrec() << ", cons: " << it_notes->getCons() << endl
        << "start: " << it_notes->getStart() << ", end: " << it_notes->getEnd() << endl
        << "front margin: "  << it_notes->getFrontMargin()
        << ", front padding: " << it_notes->getFrontPadding() << endl
        << "back padding: " << it_notes->getBackPadding()
        << ", back margin: " << it_notes->getBackMargin() << endl;
    }
    const Voice *tmp_voice = vocal_lib->getVoice(it_notes->getPronAliasString());
    if (tmp_voice == 0) {
      wcerr << L"[Nakloid::vocalization] unknown alias \"" + it_notes->getPronAliasString() + L"\" found" << endl;
    } else {
      overlapper->overlapping(tmp_voice->getUnitWaveformContainer(), make_pair(it_notes->getPronStart(), it_notes->getPronEnd()), it_notes->getFrontMargin(), it_notes->getVelocities());
    }
    // show progress
    if (++counter/notes_size>percent+0.1 && (percent=floor(counter/notes_size*10)/10.0)<1.0) {
      cout << endl << percent*100 << "%..." << endl << endl;
    }
  }
  cout << endl;
  overlapper->outputWav(score->getSongPath());
  delete overlapper;

  cout << "----- vocalization finished -----" << endl << endl;

  if (!params.path_output_score.empty()) {
    score->saveScore(params.path_output_score);
  }

  if (!params.path_output_pitches.empty()) {
    score->savePitches(params.path_output_pitches);
  }

  return true;
}

bool Nakloid::makeAllCache(bool save_pmp, bool save_uwc)
{
  if (!(save_pmp|save_uwc)) {
    return false;
  }
  cout << endl << "----- save " << ((save_pmp)?"pmp ":"") << ((save_pmp&&save_uwc)?"& ":"") << ((save_uwc)?"uwc":"") << " -----" << endl;
  if (vocal_lib != 0) {
    delete vocal_lib;
    vocal_lib = 0;
  }

  VoiceWAV::params.make_pmp_cache = save_pmp;
  VoiceWAV::params.make_uwc_cache = save_uwc;

  vocal_lib = new VocalLibrary(params.path_singer);
  if (vocal_lib==0 || !vocal_lib->initVoiceMap(true)) {
    cerr << "[Nakloid::vocalization] can't find VocalLibrary" << endl;
  }
  return true;
}

bool Nakloid::makeCache(PronunciationAlias pron_alias, bool save_pmp, bool save_uwc)
{
  if (!(save_pmp|save_uwc)) {
    return false;
  }
  wcout << L"----- save " << ((save_pmp)?L"pmp ":L"") << ((save_pmp&&save_uwc)?L"& ":L"") << ((save_uwc)?L"uwc":L"") << L" of \"" << pron_alias.getAliasString() << L"\" -----" << endl;
  if (vocal_lib != 0) {
    delete vocal_lib;
    vocal_lib = 0;
  }

  VoiceWAV::params.make_pmp_cache = save_pmp;
  VoiceWAV::params.make_uwc_cache = save_uwc;

  vocal_lib = new VocalLibrary(params.path_singer);
  if (vocal_lib==0 || !vocal_lib->makeFileCache(pron_alias)) {
    cerr << "[Nakloid::vocalization] can't find VocalLibrary" << endl;
  }
  wcout << L"succeeded to create cache of \"" << pron_alias.getAliasString() << endl;

  return true;
}

bool Nakloid::makeCache(std::wstring pron_alias, bool save_pmp, bool save_uwc)
{
  return makeCache(PronunciationAlias(pron_alias), save_pmp, save_uwc);
}
