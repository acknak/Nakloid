#include "VoiceDB.h"

using namespace std;

VoiceDB::VoiceDB():path_singer("") {}

VoiceDB::VoiceDB(string path_singer)
{
  setSingerPath(path_singer);
}

VoiceDB::~VoiceDB() {}

bool VoiceDB::initVoiceMap()
{
  namespace fs = boost::filesystem;
  const fs::path path(path_singer);

  if (fs::is_directory(path)) {
    for (fs::directory_iterator it(path), eod; it != eod; ++it) {
      if (it->path().leaf().string() == "oto.ini") {
        cout << "loading " << it->path().string() << endl;
        initVoiceMap(it->path().string());
      }
    }
    if (voice_map.size() > 0)
      return true;
  }

  cerr << "[VoiceDB::initVoiceMap] path_singer is invalid" << endl;
  return false;
 }

bool VoiceDB::initVoiceMap(string path_oto_ini)
{
  ifstream ifs(path_oto_ini.c_str());
  boost::filesystem::path path_ini(path_oto_ini);
  string buf, wav_ext=".wav";
  while(ifs && getline(ifs, buf)) {
    Voice tmp_voice;
    vector<string> v1, v2;
    boost::algorithm::split(v1, buf, boost::is_any_of("="));
    boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
    string filename = v1[0].substr(0, v1[0].find_last_of(wav_ext)-wav_ext.size()+1);
    short tmp;
    tmp_voice.path = path_ini.parent_path().string()+"/";
    tmp_voice.filename = filename;
    tmp_voice.offs = (((tmp=boost::lexical_cast<double>(v2[1]))>0))?tmp:0;
    tmp_voice.cons = (((tmp=boost::lexical_cast<double>(v2[2]))>0))?tmp:0;
    tmp_voice.blnk = boost::lexical_cast<double>(v2[3]);
    tmp_voice.prec = boost::lexical_cast<double>(v2[4]);
    tmp_voice.ovrl = boost::lexical_cast<double>(v2[5]);
    if (v1[0].find(wav_ext) == string::npos)
      tmp_voice.frq = 260.0;
    else {
      ifstream ifs_frq((tmp_voice.path+filename+"_wav.frq").c_str(), ios::binary);
      ifs_frq.seekg(sizeof(char)*12, ios_base::beg);
      ifs_frq.read((char*)&(tmp_voice.frq), sizeof(double));
    }
    // get prefix & suffix
    tmp_voice.pron = v2[0];
    tmp_voice.prefix = tmp_voice.suffix = "";
    if (tmp_voice.pron.size()>2 && tmp_voice.pron[1]==' ') {
      tmp_voice.prefix = tmp_voice.pron[0];
      tmp_voice.pron.erase(0, 2);
    }
    if (tmp_voice.pron.size() > 2) {
      string tmp_suffix = tmp_voice.pron.substr(tmp_voice.pron.size()-2, 2);
      if (isalpha(tmp_suffix[0]) && isdigit(tmp_suffix[1])) {
        tmp_voice.suffix = tmp_suffix;
        tmp_voice.pron.erase(tmp_voice.pron.size()-2, 2);
      }
    }
    // set vowel_map
    tmp_voice.is_vcv = (tmp_voice.prefix!=""&&tmp_voice.prefix!="-");
    if (!tmp_voice.is_vcv) {
      map<string, string>::const_iterator it = nak::getVow2PronIt(tmp_voice.pron);
      if (it!=nak::vow2pron.end()) {
        WavParser wav_parser(tmp_voice.path+tmp_voice.filename+".wav");
        wav_parser.addTargetTrack(0);
        if (wav_parser.parse()) {
          vector<short> tmp_wav = (*(wav_parser.getDataChunks().begin())).getDataVector();
          vector<short>::iterator it_tmp_wav_cons = tmp_wav.begin()+((tmp_voice.offs+tmp_voice.cons)/1000.0*wav_parser.getFormat().dwSamplesPerSec);
          short win_size = wav_parser.getFormat().dwSamplesPerSec / tmp_voice.frq;
          vector<short>::iterator it_tmp_wav_max = max_element(it_tmp_wav_cons, it_tmp_wav_cons+win_size);
          vowel_map[nak::pron2vow[it->second]+tmp_voice.suffix].assign(it_tmp_wav_max-(win_size/2), it_tmp_wav_max-(win_size/2)+win_size);
        }
      }
    }
    voice_map[v2[0]] = tmp_voice;
  }
  return true;
}

Voice VoiceDB::getVoice(string pron)
{
  if (!isPron(pron))
    return getNullVoice();

  if (voice_map[pron].bwc.base_wavs.empty()) {
    Voice tmp_voice = voice_map[pron];
    BaseWavsContainer bwc;
    BaseWavsFileIO *bwc_io = new BaseWavsFileIO();
    string tmp_filename = ((tmp_voice.prefix=="*")?"_":tmp_voice.prefix)+tmp_voice.pron;

    cout << "loading voice \"" << pron << "\" from ";
    if (nak::cache && bwc_io->isBaseWavsContainerFile(tmp_voice.path+tmp_filename+".bwc")) {
      cout << "cache" << endl;
      tmp_voice.bwc = ((BaseWavsContainer)bwc_io->get(tmp_voice.path+tmp_filename+".bwc"));
    } else {
      cout << "wav data" << endl;
      // get wav data
      WavParser wav_parser(tmp_voice.path+tmp_voice.filename+".wav");
      wav_parser.addTargetTrack(0);
      if (!wav_parser.parse()) {
        tmp_voice = getNullVoice();
      } else {
        vector<short> wav_data = (*(wav_parser.getDataChunks().begin())).getDataVector();
        unsigned long fs = wav_parser.getFormat().dwSamplesPerSec;

        // make input pitch mark
        PitchMarker *marker = new PitchMarker();
        marker->setInputWav(wav_data, tmp_voice.offs, tmp_voice.cons, tmp_voice.blnk, fs);
        if (nak::pron2vow.find(tmp_voice.pron) != nak::pron2vow.end()) {
          short win_size = fs / tmp_voice.frq;
          vector<short> vowel_wav = vowel_map[nak::pron2vow[tmp_voice.pron]+tmp_voice.suffix];
          if (vowel_wav.size() > win_size) {
            vowel_wav.erase(vowel_wav.begin(), vowel_wav.begin()+((vowel_wav.size()-win_size)/2));
            vowel_wav.erase(vowel_wav.end()-(vowel_wav.size()-win_size));
          } else if (vowel_wav.size() < win_size) {
            vowel_wav.insert(vowel_wav.begin(), (win_size-vowel_wav.size())/2, 0);
            vowel_wav.insert(vowel_wav.end(), win_size-vowel_wav.size(), 0);
          }
          marker->mark(vowel_wav);
        } else {
          marker->mark(tmp_voice.frq, fs);
        }
        vector<long> input_pitch_marks = marker->getMarkVector();
        delete marker;

        // make base waves
        BaseWavsMaker *maker = new BaseWavsMaker();
        maker->setPitchMarks(input_pitch_marks, tmp_voice.offs+tmp_voice.cons, tmp_voice.offs+tmp_voice.ovrl, fs);
        maker->makeBaseWavs(wav_data, tmp_voice.is_vcv);

        bwc.base_wavs = maker->getBaseWavs();
        bwc.format.wLobeSize = nak::base_wavs_lobe;
        bwc.format.dwRepeatStart = maker->getRepStartSub();
        bwc.format.wF0 = voice_map[pron].frq;
        delete maker;

        // output bwc
        if (nak::cache) {
          bwc.format.setDefaultValues();
          bwc.format.chunkSize += BaseWavsFormat::wAdditionalSize + sizeof(short);
          bwc.format.wFormatTag = BaseWavsFormat::BaseWavsFormatTag;
          bwc.format.dwSamplesPerSec = wav_parser.getFormat().dwSamplesPerSec;
          bwc_io->set(voice_map[pron].path+tmp_filename+".bwc", bwc);
        }

        voice_map[pron].bwc = bwc;
      }
    }
  }

  return voice_map[pron];
}

bool VoiceDB::isPron(string pron)
{
  return !((voice_map.empty()&&!initVoiceMap()) || voice_map.count(pron)==0);
}

void VoiceDB::setSingerPath(string path_singer)
{
  this->path_singer = path_singer;
}

string VoiceDB::getSingerPath()
{
  return this->path_singer;
}

Voice VoiceDB::getNullVoice()
{
  Voice tmp_voice = {};
  return tmp_voice;
}
