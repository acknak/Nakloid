#include "VoiceDB.h"

using namespace std;

const char VoiceDB::base_wavs_lobe = 3;

VoiceDB::VoiceDB()
{
  singer = "";
}

VoiceDB::VoiceDB(string singer)
{
  VoiceDB();
  setSinger(singer);
}

VoiceDB::~VoiceDB() {}

bool VoiceDB::initVoiceMap()
{
  return initVoiceMap("vocal/"+singer+"/oto.ini");
}

bool VoiceDB::initVoiceMap(string filename)
{
  this->singer = singer;

  ifstream ifs(filename.c_str());
  string buf, wav_ext=".wav";
  while(ifs && getline(ifs, buf)) {
    vector<string> v1, v2;
    boost::algorithm::split(v1, buf, boost::is_any_of("="));
    boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
    string filename = v1[0].substr(0, v1[0].find_last_of(wav_ext)-wav_ext.size()+1);
    if (v2[0] == "") {
      if (v2[0] == filename)
        continue;
      else
        v2[0] = filename;
    }
    short tmp;
    voice_map[v2[0]].filename = filename;
    voice_map[v2[0]].offs = (((tmp=boost::lexical_cast<short>(v2[1]))>0))?tmp:0;
    voice_map[v2[0]].cons = (((tmp=boost::lexical_cast<short>(v2[2]))>0))?tmp:0;
    voice_map[v2[0]].blnk = (((tmp=boost::lexical_cast<short>(v2[3]))>0))?tmp:0;
    voice_map[v2[0]].prec = boost::lexical_cast<short>(v2[4]);
    voice_map[v2[0]].ovrl = boost::lexical_cast<short>(v2[5]);
    voice_map[v2[0]].is_normalize = (v2.size()>6)?((tmp=boost::lexical_cast<short>(v2[6]))>0):true;
    if (v1[0].find(wav_ext) == string::npos)
      voice_map[v2[0]].frq = 260.0;
    else {
      ifstream ifs_frq(("vocal/"+singer+"/"+filename+"_wav.frq").c_str(), ios::binary);
      ifs_frq.seekg(sizeof(char)*12, ios_base::beg);
      ifs_frq.read((char*)&(voice_map[v2[0]].frq), sizeof(double));
    }
  }
  return true;
}

Voice VoiceDB::getVoice(string pron)
{
  if ((voice_map.empty()&&!initVoiceMap()) || voice_map.find(pron)==voice_map.end())
    return getNullVoice();

  if (voice_map[pron].bwc.base_wavs.empty()) {
    Voice tmp_voice = voice_map[pron];
    BaseWavsContainer bwc;
    BaseWavsFileIO *bwc_io = new BaseWavsFileIO();

    if (bwc_io->isBaseWavsContainerFile("vocal/"+singer+"/"+voice_map[pron].filename+".bwc")) {
      voice_map[pron].bwc = ((BaseWavsContainer)bwc_io->get("vocal/"+singer+"/"+voice_map[pron].filename+".bwc"));
    } else {
      // get wav data
      WavParser wav_parser("vocal/"+singer+"/"+voice_map[pron].filename+".wav");
      wav_parser.addTargetTrack(0);
      if (!wav_parser.parse()) {
        tmp_voice = getNullVoice();
      } else {
        wav_parser.normalize();
        vector<short> wav_data = (*(wav_parser.getDataChunks().begin())).getDataVector();
        unsigned long fs = wav_parser.getFormat().dwSamplesPerSec;

        wav_data.erase(wav_data.begin(), wav_data.begin()+(voice_map[pron].offs*fs/1000));
        wav_data.erase(wav_data.end()-(voice_map[pron].blnk*fs/1000), wav_data.end());

        // make input pitch mark
        PitchMarker *marker = new PitchMarker(voice_map[pron].frq, fs);
        marker->setConsPos(voice_map[pron].cons, fs);
        marker->mark(wav_data);
        vector<long> input_pitch_marks = marker->getMarkVector();
        delete marker;

        // make base waves
        BaseWavsMaker *maker = new BaseWavsMaker();
        maker->setPitchMarks(input_pitch_marks);
        maker->setVoice(wav_data);
        maker->setLobe(base_wavs_lobe);
        maker->setRepStart(voice_map[pron].cons, fs);
        maker->makeBaseWavs();

        bwc.base_wavs = maker->getBaseWavs();
        bwc.format.wLobeSize = maker->getLobe();
        bwc.format.dwRepeatStart = maker->getRepStartPoint();
        delete maker;

        // output bwc
        bwc.format.setDefaultValues();
        bwc.format.chunkSize += BaseWavsFormat::wAdditionalSize + sizeof(short);
        bwc.format.wFormatTag = BaseWavsFormat::BaseWavsFormatTag;
        bwc.format.dwSamplesPerSec = wav_parser.getFormat().dwSamplesPerSec;
        bwc_io->set("vocal/"+singer+"/"+voice_map[pron].filename+".bwc", bwc);

        voice_map[pron].bwc = bwc;
      }
    }
  }

  return voice_map[pron];
}

void VoiceDB::setSinger(string singer)
{
  this->singer = singer;
}

string VoiceDB::getSinger()
{
  return this->singer;
}

Voice VoiceDB::getNullVoice()
{
  Voice tmp_voice = {};
  return tmp_voice;
}
