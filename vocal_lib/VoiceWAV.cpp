#include "VoiceWAV.h"

using namespace std;

map< wstring, vector<double> > VoiceWAV::vowel_wav_map;

const UnitWaveformContainer* VoiceWAV::getUnitWaveformContainer() const
{
  if (uwc != 0) {
    return uwc;
  }

  uwc = new UnitWaveformContainer();
  wcout << L"load voice \"" << getPronAliasString() << L"\" from wav data" << endl;

  // get wav data
  tmp_wav.clear();
  WavParser wav_parser(path, (WavHandler*)this);
  wav_parser.parse();

  // make input pitch mark
  vector<long> input_pitchmarks;
  long sub_fade_start=0, sub_fade_end=0;
  {
    PitchMarker *marker = new PitchMarker(tmp_wav.data.getData(), offs, ovrl, prec, blnk, tmp_wav.header.dwSamplesPerSec);
    if (!pron_alias.getPronVowel().empty()) {
      short win_size = tmp_wav.header.dwSamplesPerSec / getFrq() * 2;
      vector<double> aft_vowel_wav = getVowelWav(pron_alias.getPronVowel());
      trimVector(&aft_vowel_wav, win_size);
      if (isVCV() && pron_alias.getPrefixVowel().empty()) {
        vector<double> fore_vowel_wav = getVowelWav(pron_alias.getPrefixVowel());
        trimVector(&fore_vowel_wav, win_size);
        marker->mark(fore_vowel_wav, aft_vowel_wav);
      } else {
        marker->mark(aft_vowel_wav);
      }
    } else {
      marker->mark(getFrq(), tmp_wav.header.dwSamplesPerSec);
    }
    input_pitchmarks = marker->getPitchMarks();
    sub_fade_start = marker->getFadeStartSub();
    sub_fade_end = marker->getFadeEndSub();
    delete marker;
  }

  // make unit waveforms
  {
    UnitWaveformMaker *maker = new UnitWaveformMaker(uwc, input_pitchmarks);
    maker->setOvrl(offs+ovrl, tmp_wav.header.dwSamplesPerSec);
    maker->setFadeParams(sub_fade_start, sub_fade_end);
    maker->makeUnitWaveform(tmp_wav.data.getData(), tmp_wav.header.dwSamplesPerSec/getFrq(), isVCV());
    uwc->header.wLobeSize = Voice::params.num_default_uwc_lobes;
    uwc->header.wF0 = getFrq();
    delete maker;
  }

  // output uwc
  if (params.uwc_cache) {
    uwc->header.wFormatTag = UnitWaveformHeader::UnitWaveformFormatTag;
    uwc->header.dwSamplesPerSec = tmp_wav.header.dwSamplesPerSec;
    uwc->save(path.parent_path()/boost::algorithm::replace_all_copy(pron_alias.getAliasString()+L".uwc", L"*", L"_"));
  }

  tmp_wav.clear();
  return this->uwc;
}

const vector<double>& VoiceWAV::getVowelWav(wstring vowel) const
{
  if (vowel_wav_map.count(vowel) == 0) {
    tmp_wav.clear();
    WavParser wav_parser(path, (WavHandler*)this);
    wav_parser.parse();
    short win_size = tmp_wav.header.dwSamplesPerSec / frq;
    vector<double> tmp_win(win_size*2, 0);
    if (params.num_default_uwc_lobes > 1) {
      long pos_half = tmp_win.size() / 2;
      if (tmp_win.size() % 2 > 0) {
        tmp_win[pos_half] = 1.0;
        ++pos_half;
      }
      for (size_t i = 0; i<tmp_win.size() - pos_half; i++) {
        double x = (i + 1.0) * params.num_default_uwc_lobes / pos_half;
        tmp_win[pos_half + i] = sinc(x) * sinc(x / params.num_default_uwc_lobes);
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
    vowel_wav_map[vowel].assign(it_tmp_wav_max-win_size, it_tmp_wav_max+win_size);
    tmp_wav.clear();
  }
  return vowel_wav_map[vowel];
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
