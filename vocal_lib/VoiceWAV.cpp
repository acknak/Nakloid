#include "VoiceWAV.h"

using namespace std;

struct VoiceWAV::Parameters VoiceWAV::params;
map< wstring, vector<double> > VoiceWAV::vowel_wav_map;

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
    long sub_fade_start=0, sub_fade_end=0;
    PitchMarker *marker = new PitchMarker(tmp_wav.data.getData(), offs, ovrl, prec, blnk, tmp_wav.header.dwSamplesPerSec);
    if (!pron_alias.getPronVowel().empty()) {
      short win_size = tmp_wav.header.dwSamplesPerSec / getFrq() * 2;
      vector<double> aft_vowel_wav = getVowelWav();
      vector<double> fore_vowel_wav;
      trimVector(&aft_vowel_wav, win_size);
      if (isVCV() && (fore_vowel_wav=getPrefixVowelWav()).size()>0) {
        trimVector(&fore_vowel_wav, win_size);
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
  if (Voice::params.num_default_uwc_lobes > 1) {
    long pos_half = tmp_win.size() / 2;
    if (tmp_win.size() % 2 > 0) {
      tmp_win[pos_half] = 1.0;
      ++pos_half;
    }
    for (size_t i = 0; i<tmp_win.size() - pos_half; i++) {
      double x = (i + 1.0) * Voice::params.num_default_uwc_lobes / pos_half;
      tmp_win[pos_half + i] = sinc(x) * sinc(x / Voice::params.num_default_uwc_lobes);
    }
    reverse_copy(tmp_win.begin() + pos_half, tmp_win.end(), tmp_win.begin());
  } else {
    for (size_t i = 0; i<tmp_win.size(); ++i) {
      double x = (i + 1.0) / (tmp_win.size() + 1.0);
      tmp_win[i] = 0.5 - (0.5 * cos(2 * M_PI*x));
    }
  }
  vector<double>::const_iterator it_tmp_wav_cons = tmp_wav.data.getData().begin()+((offs+cons)/1000.0*tmp_wav.header.dwSamplesPerSec);
  vector<double>::const_iterator it_tmp_wav_max = it_tmp_wav_cons;
  double tmp_max_rms = -1.0, avr_wav = accumulate(it_tmp_wav_cons-win_size,it_tmp_wav_cons+(win_size*3),0)/tmp_wav.data.getData().size();
  for (size_t i=0; i<win_size*2; i++) {
    if (*(it_tmp_wav_cons+i)<avr_wav) {
      continue;
    }
    vector<double> tmp_wav(it_tmp_wav_cons+i-win_size, it_tmp_wav_cons+i+win_size);
    for (size_t j=0; j<tmp_wav.size(); j++) {
      tmp_wav[j] *= tmp_win[j];
    }
    double tmp_rms = getRMS(tmp_wav.begin(), tmp_wav.end());
    if (tmp_rms>tmp_max_rms) {
      tmp_max_rms = tmp_rms;
      it_tmp_wav_max = it_tmp_wav_cons+i;
    }
  }
  vowel_wav_map[pron_alias.getPronVowel()+pron_alias.suffix].assign(it_tmp_wav_max-win_size, it_tmp_wav_max+win_size);
  tmp_wav.clear();
}

const vector<double>& VoiceWAV::getVowelWav() const
{
  map< wstring, vector<double> >::iterator it = vowel_wav_map.find(pron_alias.getPronVowel()+pron_alias.suffix);
  setVowelWav();
  return vowel_wav_map[pron_alias.getPronVowel()+pron_alias.suffix];
}

const vector<double>& VoiceWAV::getPrefixVowelWav() const
{
  map< wstring, vector<double> >::iterator it = vowel_wav_map.find(pron_alias.getPrefixVowel()+pron_alias.suffix);
  if (it != vowel_wav_map.end()) {
    return vowel_wav_map[pron_alias.getPrefixVowel()+pron_alias.suffix];
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
