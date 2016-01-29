#include "VoiceWAV.h"

using namespace std;

struct VoiceWAV::Parameters VoiceWAV::params;
map< wstring, VoiceWAV::PitchmarkTemplateParameters > VoiceWAV::vowel_wav_map;

const UnitWaveformContainer* VoiceWAV::getUnitWaveformContainer() const
{
  if (uwc == 0) {
    makeUnitWaveformContainerCache();
  }
  return uwc;
}

void VoiceWAV::makeUnitWaveformContainerCache(bool save_memory_cache) const
{
  UnitWaveformContainer *tmp_uwc;
  tmp_uwc = new UnitWaveformContainer();
  wcout << L"load voice \"" << getPronAliasString() << L"\" from wav data" << endl;

  // get wav data
  tmp_wav.clear();
  WavParser wav_parser(path, (WavHandler*)this);
  wav_parser.parse();

  // make input pitch mark
  PitchmarkParameters pmp;
  boost::filesystem::path path_pmp = path.parent_path()/boost::algorithm::replace_all_copy((pron_alias.getAliasString()+L".pmp"), L"*", L"_");
  boost::system::error_code err_pmp;
  const bool result_pmp = boost::filesystem::exists(path_pmp, err_pmp);
  if (!params.make_pmp_cache && (params.use_pmp_cache && result_pmp && !err_pmp)) {
    pmp.load(path_pmp);
  } else {
    PitchMarker *marker = new PitchMarker(tmp_wav.data.getData(), offs, ovrl, cons, blnk, tmp_wav.header.dwSamplesPerSec);
    if (!pron_alias.getPronVowel().empty()) {
      short win_size = tmp_wav.header.dwSamplesPerSec / getFrq() * 2;

      vector<double> aft_vowel_wav = getVowelWav();
      trimVector(&aft_vowel_wav, win_size);
      pmp.base_vowel_wav_filename = vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].filename;
      pmp.base_vowel_wav_from = vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].from;
      pmp.base_vowel_wav_to = vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].to;

      vector<double> fore_vowel_wav;
      if (isVCV() && (fore_vowel_wav=getPrefixVowelWav()).size()>0) {
        trimVector(&fore_vowel_wav, win_size);
        pmp.prefix_vowel_wav_filename = vowel_wav_map[pron_alias.getPrefixVowel() + pron_alias.suffix].filename;
        pmp.prefix_vowel_wav_from = vowel_wav_map[pron_alias.getPrefixVowel() + pron_alias.suffix].from;
        pmp.prefix_vowel_wav_to = vowel_wav_map[pron_alias.getPrefixVowel() + pron_alias.suffix].to;
        marker->mark(fore_vowel_wav, aft_vowel_wav);
      } else {
        marker->mark(aft_vowel_wav);
      }
    } else {
      marker->mark(getFrq(), tmp_wav.header.dwSamplesPerSec);
    }
    pmp.pitchmark_points = marker->getPitchMarks();
    pmp.sub_fade_start = marker->getFadeStartSub();
    delete marker;

    pmp.filename = path.filename().wstring();
    pmp.base_pitch = tmp_wav.header.dwSamplesPerSec/getFrq();
    if (params.make_pmp_cache) {
      pmp.save(path_pmp);
    }
  }

  // make unit waveforms
  {
    UnitWaveformMaker *maker = new UnitWaveformMaker(tmp_uwc, pmp.pitchmark_points);
    maker->setOvrl(offs+ovrl, tmp_wav.header.dwSamplesPerSec);
    maker->setFadeParams(pmp.sub_fade_start, pmp.pitchmark_points.size()-1);
    maker->makeUnitWaveform(tmp_wav.data.getData(), tmp_wav.header.dwSamplesPerSec/getFrq(), isVCV());
    tmp_uwc->header.wLobeSize = Voice::params.num_default_uwc_lobes;
    tmp_uwc->header.wF0 = getFrq();
    delete maker;
  }

  // output uwc
  if (params.make_uwc_cache) {
    tmp_uwc->header.wFormatTag = UnitWaveformHeader::UnitWaveformFormatTag;
    tmp_uwc->header.dwSamplesPerSec = tmp_wav.header.dwSamplesPerSec;
    tmp_uwc->save(path.parent_path()/boost::algorithm::replace_all_copy(pron_alias.getAliasString()+L".uwc", L"*", L"_"));
  }

  tmp_wav.clear();

  if (save_memory_cache) {
    if (uwc != 0) {
      delete uwc;
      uwc = 0;
    }
    uwc = tmp_uwc;
  } else {
    delete tmp_uwc;
    tmp_uwc = 0;
  }
}

void VoiceWAV::setVowelWav() const
{
  tmp_wav.clear();
  WavParser wav_parser(path, (WavHandler*)this);
  wav_parser.parse();
  short win_size = tmp_wav.header.dwSamplesPerSec / getFrq();
  vector<double> tmp_win(win_size*2, 0);
  vector<double>::const_iterator tmp_pitchmark = tmp_wav.data.getData().begin() + ((offs+cons)/1000.0*tmp_wav.header.dwSamplesPerSec);
  vector<double> xcorr_win(win_size * 2, 0.0);
  vector<double> base_wave(win_size, 0.0);
  for (size_t i = 0; i<win_size; i++) {
    base_wave[i] = sin((i*2*M_PI/win_size) + (3.0/4.0*2*M_PI));
  }
  long dist = win_size / 2;
  double xcorr_sum = 0, xcorr_start = 0;
  xcorr(tmp_pitchmark, xcorr_win.begin(), (vector<double>::const_iterator)base_wave.begin(), (vector<double>::const_iterator)base_wave.end());
  short margin_fore, margin_aft;
  dist = max_element(xcorr_win.begin(), xcorr_win.end()) - xcorr_win.begin();
  vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].waveform.assign(tmp_pitchmark+dist-win_size, tmp_pitchmark+dist+win_size);
  vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].from = (tmp_pitchmark+dist-tmp_wav.data.getData().begin()) - win_size;
  vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].to = (tmp_pitchmark+dist-tmp_wav.data.getData().begin()) + win_size;
  vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].filename = path.relative_path().wstring();
  tmp_wav.clear();
}

const vector<double>& VoiceWAV::getVowelWav() const
{
  map< wstring, VoiceWAV::PitchmarkTemplateParameters >::iterator it = vowel_wav_map.find(pron_alias.getPronVowel() + pron_alias.suffix);
  if (it == vowel_wav_map.end()) {
    setVowelWav();
  }
  return vowel_wav_map[pron_alias.getPronVowel() + pron_alias.suffix].waveform;
}

const vector<double>& VoiceWAV::getPrefixVowelWav() const
{
  map< wstring, VoiceWAV::PitchmarkTemplateParameters >::iterator it = vowel_wav_map.find(pron_alias.getPrefixVowel() + pron_alias.suffix);
  if (it != vowel_wav_map.end()) {
    return vowel_wav_map[pron_alias.getPrefixVowel()+pron_alias.suffix].waveform;
  }
  return vector<double>(0,0);
}

/*
 * inherit from WavHandler
 */
void VoiceWAV::chunkHeader(WavHeader wav_header)
{
  tmp_wav.header = wav_header;
}

void VoiceWAV::chunkData(WavData wav_data)
{
  if (tmp_wav.data.getData().size() == 0) {
    tmp_wav.data = wav_data;
  }
}
