#include "VoiceDB.h"

using namespace std;

VoiceDB::VoiceDB()
{
  path_singer = "";
  /*
  pron2vow = boost::assign::map_list_of
    ("あ", 'a')("い", 'i')("う", 'u')("え", 'e')("お", 'o')
    ("か", 'a')("き", 'i')("く", 'u')("け", 'e')("こ", 'o')
    ("が", 'a')("ぎ", 'i')("ぐ", 'u')("げ", 'e')("ご", 'o')
    ("さ", 'a')("し", 'i')("す", 'u')("せ", 'e')("そ", 'o')
    ("ざ", 'a')("じ", 'i')("ず", 'u')("ぜ", 'e')("ぞ", 'o')
    ("た", 'a')("ち", 'i')("つ", 'u')("て", 'e')("と", 'o')
    ("だ", 'a')("ぢ", 'i')("づ", 'u')("で", 'e')("ど", 'o')
    ("な", 'a')("に", 'i')("ぬ", 'u')("ね", 'e')("の", 'o')
    ("は", 'a')("ひ", 'i')("ふ", 'u')("へ", 'e')("ほ", 'o')
    ("ば", 'a')("び", 'i')("ぶ", 'u')("べ", 'e')("ぼ", 'o')
    ("ぱ", 'a')("ぴ", 'i')("ぷ", 'u')("ぺ", 'e')("ぽ", 'o')
    ("ま", 'a')("み", 'i')("む", 'u')("め", 'e')("も", 'o')
    ("や", 'a')("ゆ", 'u')("よ", 'o')("ゃ", 'a')("ゅ", 'u')("ょ", 'o')
    ("ら", 'a')("り", 'i')("る", 'u')("れ", 'e')("ろ", 'o')
    ("わ", 'a')("を", 'o')("ん", 'n')("ゐ", 'i')("ゑ", 'e');
    */
}

VoiceDB::VoiceDB(string path_singer)
{
  VoiceDB();
  setSingerPath(path_singer);
}

VoiceDB::~VoiceDB() {}

bool VoiceDB::initVoiceMap()
{
  namespace fs = boost::filesystem;
  const fs::path path(path_singer);

  if (fs::is_directory(path)) {
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(path), fs::recursive_directory_iterator())) {
      if (p.leaf().string() == "oto.ini")
        initVoiceMap(p.string());
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
    vector<string> v1, v2;
    boost::algorithm::split(v1, buf, boost::is_any_of("="));
    boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
    string filename = v1[0].substr(0, v1[0].find_last_of(wav_ext)-wav_ext.size()+1);
    string pron = (filename==v2[0]||v2[0]==""||voice_map.count(filename)==0)?filename:v2[0];
    short tmp;
    voice_map[pron].path = path_ini.parent_path().string();
    voice_map[pron].filename = filename;
    voice_map[pron].offs = (((tmp=boost::lexical_cast<double>(v2[1]))>0))?tmp:0;
    voice_map[pron].cons = (((tmp=boost::lexical_cast<double>(v2[2]))>0))?tmp:0;
    voice_map[pron].blnk = boost::lexical_cast<double>(v2[3]);
    voice_map[pron].prec = boost::lexical_cast<double>(v2[4]);
    voice_map[pron].ovrl = boost::lexical_cast<double>(v2[5]);
    if (v1[0].find(wav_ext) == string::npos)
      voice_map[pron].frq = 260.0;
    else {
      ifstream ifs_frq((path_ini.parent_path().string()+"/"+filename+"_wav.frq").c_str(), ios::binary);
      ifs_frq.seekg(sizeof(char)*12, ios_base::beg);
      ifs_frq.read((char*)&(voice_map[pron].frq), sizeof(double));
    }
    // alias
    if (v2[0]!="" && v2[0]!=pron)
      voice_map[v2[0]] = voice_map[pron];
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
    string tmp_filename = pron;
    if (voice_map[pron].filename != pron)
      if (tmp_filename[1] == ' ')
        if (tmp_filename[0] == '*')
          tmp_filename.replace(0, 2, "_");
        else if (tmp_filename[0] == '-')
          tmp_filename.replace(0, 2, "-");

    if (nak::cache && bwc_io->isBaseWavsContainerFile(voice_map[pron].path+"/"+tmp_filename+".bwc")) {
      voice_map[pron].bwc = ((BaseWavsContainer)bwc_io->get(voice_map[pron].path+"/"+tmp_filename+".bwc"));
    } else {
      // get wav data
      WavParser wav_parser(voice_map[pron].path+"/"+voice_map[pron].filename+".wav");
      wav_parser.addTargetTrack(0);
      if (!wav_parser.parse()) {
        tmp_voice = getNullVoice();
      } else {
        vector<short> wav_data = (*(wav_parser.getDataChunks().begin())).getDataVector();
        unsigned long fs = wav_parser.getFormat().dwSamplesPerSec;

        // make input pitch mark
        PitchMarker *marker = new PitchMarker(voice_map[pron].frq, fs);
        marker->setRange(voice_map[pron].offs, voice_map[pron].cons, voice_map[pron].blnk, fs);
        marker->mark(wav_data);
        vector<long> input_pitch_marks = marker->getMarkVector();
        delete marker;

        // make base waves
        BaseWavsMaker *maker = new BaseWavsMaker();
        maker->setPitchMarks(input_pitch_marks);
        maker->setVoice(wav_data);
        maker->setRepStart(voice_map[pron].offs+voice_map[pron].cons, fs);
        maker->makeBaseWavs();

        bwc.base_wavs = maker->getBaseWavs();
        bwc.format.wLobeSize = maker->getLobe();
        bwc.format.dwRepeatStart = maker->getRepStartPoint();
        bwc.format.wF0 = voice_map[pron].frq;
        delete maker;

        // output bwc
        if (nak::cache) {
          bwc.format.setDefaultValues();
          bwc.format.chunkSize += BaseWavsFormat::wAdditionalSize + sizeof(short);
          bwc.format.wFormatTag = BaseWavsFormat::BaseWavsFormatTag;
          bwc.format.dwSamplesPerSec = wav_parser.getFormat().dwSamplesPerSec;
          bwc_io->set(voice_map[pron].path+"/"+tmp_filename+".bwc", bwc);
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
