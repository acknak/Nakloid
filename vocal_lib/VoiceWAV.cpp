#include "VoiceWAV.h"

using namespace std;

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
  {
    PitchMarker *marker = new PitchMarker(tmp_wav.data.getData(), offs, ovrl, prec, blnk, tmp_wav.header.dwSamplesPerSec);
    if (PronunciationAlias::isVowel(pron_alias.getVowel())) {
      short win_size = tmp_wav.header.dwSamplesPerSec / getFrq() * 2;
      vector<double> aft_vowel_wav = getVowelWav();
      trimVector(&aft_vowel_wav, win_size);
      if (is_vcv && vowel_wav_map.count(pron_alias.pron)>0) {
        vector<double> fore_vowel_wav = vowel_wav_map[pron_alias.pron];
        trimVector(&fore_vowel_wav, win_size);
        marker->mark(fore_vowel_wav, aft_vowel_wav);
      } else {
        marker->mark(aft_vowel_wav);
      }
    } else {
      marker->mark(getFrq(), tmp_wav.header.dwSamplesPerSec);
    }
    input_pitchmarks = marker->getPitchMarks();
    delete marker;
  }

  // make unit waveforms
  {
    UnitWaveformMaker *maker = new UnitWaveformMaker();
    maker->setPitchMarks(input_pitchmarks, offs+cons, offs+ovrl, tmp_wav.header.dwSamplesPerSec);
    maker->makeUnitWaveform(tmp_wav.data.getData(), tmp_wav.header.dwSamplesPerSec/getFrq(), is_vcv);
    uwc->unit_waveforms = maker->getUnitWaveform();
    uwc->header.wLobeSize = Voice::params.num_default_uwc_lobes;
    uwc->header.dwRepeatStart = maker->getFadeStartSub();
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

const vector<double>& VoiceWAV::getVowelWav() const
{
  if (vowel_wav_map.count(pron_alias.getVowel()) == 0) {
    tmp_wav.clear();
    WavParser wav_parser(path, (WavHandler*)this);
    wav_parser.parse();
    short win_size = tmp_wav.header.dwSamplesPerSec / frq;
    vector<double> tmp_win = nak::getWindow(win_size*2, params.num_default_uwc_lobes);
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
      double tmp_rms = nak::getRMS(tmp_wav);
      if (tmp_rms>tmp_max_rms) {
        tmp_max_rms = tmp_rms;
        it_tmp_wav_max = it_tmp_wav_cons+i;
      }
    }
    vowel_wav_map[pron_alias.getVowel()].assign(it_tmp_wav_max-win_size, it_tmp_wav_max+win_size);
    tmp_wav.clear();
  }
  return vowel_wav_map[pron_alias.getVowel()];
}

const bool VoiceWAV::hasVowelWav() const
{
  if (vowel_wav_map.count(pron_alias.getVowel()) == 0) {
    tmp_wav.clear();
    WavParser wav_parser(path, (WavHandler*)this);
    wav_parser.parse();
    short win_size = tmp_wav.header.dwSamplesPerSec / frq;
    vector<double> tmp_win = nak::getWindow(win_size*2, params.num_default_uwc_lobes);
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
      double tmp_rms = nak::getRMS(tmp_wav);
      if (tmp_rms>tmp_max_rms) {
        tmp_max_rms = tmp_rms;
        it_tmp_wav_max = it_tmp_wav_cons+i;
      }
    }
    vowel_wav_map[pron_alias.getVowel()].assign(it_tmp_wav_max-win_size, it_tmp_wav_max+win_size);
    tmp_wav.clear();
  }
  return true;
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
