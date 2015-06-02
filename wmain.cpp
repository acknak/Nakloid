#include <cstdio>
#include <iostream>
#include <locale>
#include <string>
#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "core/UnitWaveformOverlapper.h"
#include "score/Score.h"
#include "vocal_lib/Voice.h"
#include "vocal_lib/VoiceWAV.h"
#include "vocal_lib/VocalLibrary.h"
#include "Nakloid.h"

using namespace std;

bool parse_ini();
bool is_logging = true;

int wmain(int argc, wchar_t *argv[])
{
  // set locale
	ios_base::sync_with_stdio(false);
	locale default_loc("");
	locale::global(default_loc);
	locale ctype_default(locale::classic(), default_loc, locale::ctype);
	wcout.imbue(ctype_default);
	wcerr.imbue(ctype_default);
	wcin.imbue(ctype_default);

  Nakloid *nakloid = 0;

  if (argc > 1) {
    if (!parse_ini()) {
      cerr << "[wmain] can't parse Nakloid.ini correctly." << endl;
      return 1;
    }
    wstring mode = argv[1];
    nakloid = new Nakloid();
    if (mode == L"-v") { //vocalization
      nakloid->vocalization();
    } else if (mode == L"-ac") { //all cache
      bool make_pmp=false, make_uwc=false;
      for (size_t i=2; i<argc; i++) {
        wstring tmp_str = argv[i];
        make_pmp |= (tmp_str==L"-pmp");
        make_uwc |= (tmp_str==L"-uwc");
      }
      nakloid->makeAllCache(make_pmp, make_uwc);
    } else if (mode == L"-c") { //cache
      wstring pron_alias = argv[2];
      bool make_pmp=false, make_uwc=false;
      for (size_t i=3; i<argc; i++) {
        wstring tmp_str = argv[i];
        make_pmp |= (tmp_str==L"-pmp");
        make_uwc |= (tmp_str==L"-uwc");
      }
      nakloid->makeCache(pron_alias, make_pmp, make_uwc);
    } else {
      cerr << "[wmain] invalid option found at command line" << endl;
    }
    delete nakloid;
  } else {
    if (parse_ini()) {
      nakloid = new Nakloid();
    } else {
      cerr << "[wmain] can't parse Nakloid.ini correctly" << endl;
      cin.sync();
      cout << "Press Enter/Return to continue..." << endl;
      cin.get();
      return 0;
    }
    while(1) {
      wcout << "----- choose Nakloid mode -----" << endl
        << L"'1': vocalize" << endl
        << L"'2': make all cache of vocal library" << endl
        << L"'3': make specific cache of vocal library" << endl
        << L"'0': exit" << endl << endl;
      char input_key;
      cin >> input_key;
      if (input_key == '0') {
        delete nakloid;
        nakloid = 0;
        return 0;
      }
      if (!parse_ini()) {
        cerr << "[wmain] can't parse Nakloid.ini correctly." << endl;
        return 1;
      }
      bool make_pmp, make_uwc;
      switch(input_key) {
       case '1':
        nakloid->vocalization();
        break;
       case '2':
        cin.sync();
        wcout << "make pmp file [y/n]:";
        input_key = cin.get();
        make_pmp = (input_key=='y')?true:false;
        cin.sync();
        wcout << "make uwc file [y/n]:";
        input_key = cin.get();
        make_uwc = (input_key=='y')?true:false;
        nakloid->makeAllCache(make_pmp, make_uwc);
        break;
       case '3':
        wstring pron_alias;
        cin.sync();
        wcout << "pron_alias: ";
        getline(wcin, pron_alias);
        cin.sync();
        wcout << "make pmp file [y/n]:";
        input_key = cin.get();
        make_pmp = (input_key=='y')?true:false;
        cin.sync();
        wcout << "make uwc file [y/n]:";
        input_key = cin.get();
        make_uwc = (input_key=='y')?true:false;
        nakloid->makeCache(pron_alias, make_pmp, make_uwc);
        break;
      }
      wcout << endl;
    }
  }

  return 0;
}

bool parse_ini()
{
  bool parse_error = false;
  {
    boost::property_tree::wptree wpt;
    try {
      boost::filesystem::path fs_path_ini(L"Nakloid.ini");
      boost::property_tree::ini_parser::read_ini(fs_path_ini.string(), wpt);
    } catch (boost::property_tree::ini_parser::ini_parser_error &e) {
      cerr << "[Nakloid.ini line " << e.line() << "] " << e.message() << endl
        << "[Nakloid::Nakloid] can't parse Nakloid.ini" << endl;
      parse_error = true;
    } catch (...) {
      cerr << "[Nakloid::Nakloid] can't parse Nakloid.ini" << endl;
      parse_error = true;
    }

    if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_input_score")) {
      Nakloid::params.path_input_score = tmp.get();
    }
    {
      wstring tmp_score = wpt.get<wstring>(L"Input.score_mode", L"nak");
      if (tmp_score == L"nak") {
        Nakloid::params.score_mode = Nakloid::score_mode_nak;
      } else if (tmp_score == L"ust") {
        Nakloid::params.score_mode = Nakloid::score_mode_ust;
      } else if (tmp_score == L"smf") {
        Nakloid::params.score_mode = Nakloid::score_mode_smf;
        if (boost::optional<short> tmp = wpt.get_optional<short>(L"Input.track")) {
          Score::params.smf_track = tmp.get();
        }
        if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_lyrics")) {
          Nakloid::params.path_lyrics = tmp.get();
        }
      } else {
        cerr << "[Nakloid::Nakloid] can't recognize score_mode" << endl;
        parse_error = true;
      }
    }
    if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_input_pitches")) {
      Nakloid::params.path_input_pitches = tmp.get();
    }
    {
      wstring tmp_pitches = wpt.get<wstring>(L"Input.pitches_mode", L"");
      if (tmp_pitches == L"pit") {
        Nakloid::params.pitch_mode = Nakloid::pitches_mode_pit;
      } else if (tmp_pitches == L"lf0") {
        Nakloid::params.pitch_mode = Nakloid::pitches_mode_lf0;
        if (boost::optional<short> tmp = wpt.get_optional<short>(L"Input.pitch_frame_length")) {
          Score::params.pitch_frame_length = tmp.get();
        }
      } else {
        Nakloid::params.pitch_mode = Nakloid::pitches_mode_none;
      }
    }
    if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_singer")) {
      Nakloid::params.path_singer = tmp.get();
    }
    if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Input.path_prefix_map")) {
      Nakloid::params.path_prefix_map = tmp.get();
    }
    if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Output.path_song")) {
      Nakloid::params.path_song = tmp.get();
    }
    if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Output.path_output_score")) {
      Nakloid::params.path_output_score = tmp.get();
    }
    if (boost::optional<wstring> tmp = wpt.get_optional<wstring>(L"Output.path_output_pitches")) {
      Nakloid::params.path_output_pitches = tmp.get();
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
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"VocalLibrary.use_pmp_cache")) {
      VoiceWAV::params.use_pmp_cache = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"VocalLibrary.use_uwc_cache")) {
      VocalLibrary::params.use_uwc_cache = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Output.print_debug")) {
      Nakloid::params.print_debug = tmp.get();
    }
    if (boost::optional<long> tmp = wpt.get_optional<long>(L"UnitWaveformContainer.target_rms")) {
      UnitWaveformMaker::params.target_rms = tmp.get();
    }
    if (boost::optional<short> tmp = wpt.get_optional<short>(L"UnitWaveformContainer.num_lobes")) {
      UnitWaveformMaker::params.num_lobes = UnitWaveformOverlapper::params.num_lobes = VocalLibrary::params.num_default_uwc_lobes = Voice::params.num_default_uwc_lobes = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"UnitWaveformContainer.uwc_normalize")) {
      UnitWaveformMaker::params.normalize = tmp.get();
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
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Arrange.finefluctuation")) {
      Score::params.finefluctuation = tmp.get();
    }
    if (boost::optional<double> tmp = wpt.get_optional<double>(L"Arrange.finefluctuation_deviation")) {
      Score::params.finefluctuation_deviation = tmp.get();
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
    if (boost::optional<long> tmp = wpt.get_optional<long>(L"Pitchmark.pitch_default")) {
      Voice::params.pitch_default = tmp.get();
    }
    if (boost::optional<short> tmp = wpt.get_optional<short>(L"Pitchmark.pitch_margin")) {
      PitchMarker::params.pitch_margin = tmp.get();
    }
    if (boost::optional<double> tmp = wpt.get_optional<double>(L"Pitchmark.xcorr_threshold")) {
      PitchMarker::params.xcorr_threshold = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Overlap.stretch_self_fade")) {
      UnitWaveformOverlapper::params.stretch_self_fade = tmp.get();
    }
    if (boost::optional<long> tmp = wpt.get_optional<long>(L"Overlap.ms_self_fade")) {
      UnitWaveformOverlapper::params.ms_self_fade = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Overlap.interpolation")) {
      UnitWaveformOverlapper::params.interpolation = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Overlap.overlap_normalize")) {
      UnitWaveformOverlapper::params.overlap_normalize = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Overlap.window_modification")) {
      UnitWaveformOverlapper::params.window_modification = tmp.get();
    }
    if (boost::optional<bool> tmp = wpt.get_optional<bool>(L"Output.print_log")) {
      is_logging = tmp.get();
      if (!is_logging) {
        freopen("", "r", stdout);
        freopen("", "r", stderr);
      }
    }
  }
  return !parse_error;
}
