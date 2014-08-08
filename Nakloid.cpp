#include "Nakloid.h"

using namespace std;

Nakloid::Nakloid(wstring path_ini)
  :vocal_lib(0), score(0), path_input_score(L""), path_lyrics(L""), path_input_pitches(L""), path_singer(L""), path_prefix_map(L""),
   path_song(L""),path_output_score(L""),path_output_pitches(L""), print_log(L"")
{
  boost::property_tree::wptree wpt;
  try {
    boost::filesystem::path fs_path_ini(path_ini);
    boost::property_tree::ini_parser::read_ini(fs_path_ini.string(), wpt);
  } catch (boost::property_tree::ini_parser::ini_parser_error &e) {
    cerr << "[Nakloid.ini line " << e.line() << "] " << e.message() << endl
      << "[Nakloid::Nakloid] can't parse Nakloid.ini" << endl;
    return;
  } catch (...) {
    cerr << "[Nakloid::Nakloid] can't parse Nakloid.ini" << endl;
  }

  if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_input_score")) {
    path_input_score = tmp.get();
  }
  {
    wstring tmp_score = wpt.get<wstring>(L"Input.score_mode", L"nak");
    if (tmp_score == L"nak") {
      score_mode = score_mode_nak;
    } else if (tmp_score == L"ust") {
      score_mode = score_mode_ust;
    } else if (tmp_score == L"smf") {
      score_mode = score_mode_smf;
      if (boost::optional<short> tmp = wpt.get_optional<short>(L"Input.track")) {
        Score::params.smf_track = tmp.get();
      }
      if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_lyrics")) {
        path_lyrics = tmp.get();
      }
    } else {
      cerr << "[Nakloid::Nakloid] can't recognize score_mode" << endl;
      return;
    }
  }
  if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_input_pitches")) {
    path_input_pitches = tmp.get();
  }
  {
    wstring tmp_pitches = wpt.get<wstring>(L"Input.pitches_mode", L"");
    if (tmp_pitches == L"pit") {
      pitch_mode = pitches_mode_pit;
    } else if (tmp_pitches == L"lf0") {
      pitch_mode = pitches_mode_lf0;
      if (boost::optional<short> tmp = wpt.get_optional<short>(L"Input.pitch_frame_length")) {
        Score::params.pitch_frame_length = tmp.get();
      }
    } else {
      pitch_mode = pitches_mode_none;
    }
  }
  if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_singer")) {
    path_singer = tmp.get();
  }
  if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_prefix_map")) {
    path_prefix_map = tmp.get();
  }
  if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Output.path_song")) {
    path_song = tmp.get();
  }
  if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Output.path_output_score")) {
    path_output_score = tmp.get();
  }
  if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Output.path_output_pitches")) {
    path_output_pitches = tmp.get();
  }
  if (boost::optional<long> tmp = wpt.get_optional<long>(L"Output.ms_margin")) {
    Score::params.ms_margin = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Output.max_volume")) {
    double tmp_max_volume = tmp.get();
    if (tmp_max_volume > 1.0) {
      tmp_max_volume = 1.0;
    } else if(tmp_max_volume < 0.0) {
      tmp_max_volume = 0.0;
    }
    UnitWaveformOverlapper::params.max_volume = tmp_max_volume;
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Output.compressor")) {
    UnitWaveformOverlapper::params.compressor = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Output.compressor_threshold")) {
    UnitWaveformOverlapper::params.compressor_threshold = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Output.compressor_ratio")) {
    UnitWaveformOverlapper::params.compressor_ratio = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Output.uwc_cache")) {
    Voice::params.uwc_cache = VocalLibrary::params.uwc_cache = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Output.print_log")) {
    print_log = tmp.get();
  }
  if (boost::optional<long> tmp = wpt.get_optional<long>(L"UnitWaveformContainer.target_rms")) {
    UnitWaveformMaker::params.target_rms = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"UnitWaveformContainer.num_lobes")) {
    UnitWaveformMaker::params.num_lobes = UnitWaveformOverlapper::params.num_lobes = VocalLibrary::params.num_default_uwc_lobes = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"UnitWaveformContainer.uwc_normalize")) {
    UnitWaveformMaker::params.normalize = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"Pitchmark.pitch_margin")) {
    PitchMarker::params.pitch_margin = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Pitchmark.xcorr_threshold")) {
    PitchMarker::params.xcorr_threshold = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Overlap.fade_stretch")) {
    UnitWaveformOverlapper::params.fade_stretch = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Overlap.interpolation")) {
    UnitWaveformOverlapper::params.interpolation = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Overlap.overlap_normalize")) {
    UnitWaveformOverlapper::params.overlap_normalize = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"Note.ms_front_padding")) {
    Note::params.ms_front_padding = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"Note.ms_back_padding")) {
    Note::params.ms_back_padding = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Arrange.auto_vowel_combining")) {
    Note::params.auto_vowel_combining = Score::params.auto_vowel_combining = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Arrange.vowel_combining_volume")) {
    Note::params.vowel_combining_volume = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Arrange.vibrato")) {
    Score::params.vibrato = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"Arrange.ms_vibrato_offset")) {
    Score::params.ms_vibrato_offset = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"Arrange.ms_vibrato_width")) {
    Score::params.ms_vibrato_width = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Arrange.pitch_vibrato")) {
    Score::params.pitch_vibrato = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Arrange.overshoot")) {
    Score::params.overshoot = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"Arrange.ms_overshoot")) {
    Score::params.ms_overshoot = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Arrange.pitch_overshoot")) {
    Score::params.pitch_overshoot = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Arrange.preparation")) {
    Score::params.preparation = tmp.get();
  }
  if (boost::optional<short> tmp = wpt.get_optional<short>(L"Arrange.ms_preparation")) {
    Score::params.ms_preparation = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Arrange.pitch_preparation")) {
    Score::params.pitch_preparation = tmp.get();
  }
  if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Arrange.finefluctuation")) {
    Score::params.finefluctuation_deviation = tmp.get();
  }
  if (boost::optional<double> tmp = wpt.get_optional<double>(L"Arrange.finefluctuation_deviation")) {
    Score::params.finefluctuation_deviation = tmp.get();
  }
}

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
  cout << "----- load voice DB -----" << endl;
  if (vocal_lib != 0) {
    delete vocal_lib;
    vocal_lib = 0;
  }
  vocal_lib = new VocalLibrary(path_singer);
  if (vocal_lib==0 || !vocal_lib->initVoiceMap()) {
    cerr << "[Nakloid::vocalization] can't find VocalLibrary" << endl;
    return false;
  }

  // load score
  cout << endl << "----- load score -----" << endl;
  switch(score_mode){
  case score_mode_nak:
    score=new ScoreNAK(path_input_score, vocal_lib, path_song); break;
  case score_mode_ust:
    score=new ScoreUST(path_input_score, vocal_lib, path_song); break;
  case score_mode_smf:
    score=new ScoreSMF(path_input_score, vocal_lib, path_song, path_lyrics); break;
  }
  if (!path_prefix_map.empty()) {
    score->loadModifierMap(path_prefix_map);
    cout << "use modifier map..." << endl;
  }
  score->load();
  switch(pitch_mode){
  case pitches_mode_pit:
    score->loadPitPitches(path_input_pitches); break;
  case pitches_mode_lf0:
    score->loadLf0Pitches(path_input_pitches); break;
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
    wcout << L"synthesize \"" << it_notes->getPronAliasString() << L"\" from " << it_notes->getPronStart() << L"ms to " << it_notes->getPronEnd() << L"ms" << endl;
    /*
    cout << "ovrl: " << it_notes->getOvrl() << ", prec: " << it_notes->getPrec() << ", cons: " << it_notes->getCons() << endl
      << "start: " << it_notes->getStart() << ", end: " << it_notes->getEnd() << endl
      << "front margin: "  << it_notes->getPronStart()+it_notes->getFrontMargin()
      << ", front padding: " << it_notes->getPronStart()+it_notes->getFrontMargin()+it_notes->getFrontPadding() << endl
      << "back padding: " << it_notes->getPronEnd()-it_notes->getBackPadding()-it_notes->getBackMargin()
      << ", back margin: " << it_notes->getPronEnd()-it_notes->getBackMargin() << endl;
    */
    if (vocal_lib->isAlias(it_notes->getPronAliasString())) {
      overlapper->overlapping(vocal_lib->getVoice(it_notes->getPronAliasString())->getUnitWaveformContainer(), make_pair(it_notes->getPronStart(), it_notes->getPronEnd()), it_notes->getFrontMargin(), it_notes->getVelocities());

      // show progress
      if (++counter/notes_size>percent+0.1 && (percent=floor(counter/notes_size*10)/10.0)<1.0) {
        cout << endl << percent*100 << "%..." << endl << endl;
      }
    }
  }
  cout << endl;
  overlapper->outputWav(score->getSongPath());
  delete overlapper;

  cout << "----- vocalization finished -----" << endl << endl;

  if (!path_output_score.empty()) {
    score->saveScore(path_output_score);
  }

  if (!path_output_pitches.empty()) {
    score->savePitches(path_output_pitches);
  }

  return true;
}

bool Nakloid::is_logging() {
  return print_log;
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

  Nakloid *nakloid = new Nakloid(L"Nakloid.ini");
  if (!nakloid->is_logging()) {
    freopen("", "r", stdout);
    freopen("", "r", stderr);
  }
  nakloid->vocalization();

  delete nakloid;

  if (nakloid->is_logging()) {
    cin.sync();
    cout << "Press Enter/Return to continue..." << endl;
    cin.get();
  }

  return 0;
}
