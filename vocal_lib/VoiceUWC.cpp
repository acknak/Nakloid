#include "VoiceUWC.h"

using namespace std;

VoiceUWC::~VoiceUWC()
{
  if (uwc != 0) {
    delete uwc;
    uwc = 0;
  }
}

const UnitWaveformContainer* VoiceUWC::getUnitWaveformContainer() const
{
  if (UnitWaveformContainer::isUwcFormatFile(path)) {
    wcout << L"load voice \"" << getPronAliasString() << L"\" from uwc data" << endl;
    if (uwc == 0) {
      uwc = new UnitWaveformContainer();
      WavParser wav_parser(path, (WavHandler*)this);
      wav_parser.parse();
    }
  } else {
    wcerr << L"[VoiceUWC::getUnitWaveformContainer] can't load \"" << getPronAliasString() << L"\"" << endl;
  }
  return uwc;
}

void VoiceUWC::chunkHeader(WavHeader wav_header)
{
  uwc->header.wFormatTag = wav_header.wFormatTag;
  uwc->header.wChannels = wav_header.wChannels;
  uwc->header.dwSamplesPerSec = wav_header.dwSamplesPerSec;
  uwc->header.dwAvgBytesPerSec = wav_header.dwAvgBytesPerSec;
  uwc->header.wBlockAlign = wav_header.wBlockAlign;
  uwc->header.wBitsPerSamples = wav_header.wBitsPerSamples;
}

void VoiceUWC::chunkHeaderAdditionalField(short chunk_size, const unsigned char* const data)
{
  if (chunk_size == 10) {
    uwc->header.wLobeSize = (data[1] << 8) | data[0];
    uwc->header.dwRepeatStart = (data[5] << 24) | (data[4] << 16) | (data[3] << 8) | data[2];
	unsigned long l_tmp_wF0 = (data[9] << 24) | (data[8] << 16) | (data[7] << 8) | data[6];
	float *f_tmp_wF0 = reinterpret_cast<float*>(&l_tmp_wF0);
	uwc->header.wF0 = *f_tmp_wF0;
  }
}

void VoiceUWC::chunkData(WavData wav_data)
{
  if (uwc->unit_waveforms.size() > 0) {
    uwc->unit_waveforms.back().data = wav_data;
  }
}

void VoiceUWC::chunkFact(long chunk_size, const unsigned char* const data)
{
  UnitWaveform tmp_unit_waveform;
  if (chunk_size == UnitWaveform::const_chunk_size) {
    tmp_unit_waveform.dwPitchLeft = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0]; 
    tmp_unit_waveform.dwPitchRight = (data[7] << 24) | (data[6] << 16) | (data[5] << 8) | data[4]; 
    tmp_unit_waveform.dwPosition = (data[11] << 24) | (data[10] << 16) | (data[9] << 8) | data[8]; 
    uwc->unit_waveforms.push_back(tmp_unit_waveform);
    return;
  }
  cerr << "[VoiceUWC::chunkFact] invalid fact chunk" << endl;
}
