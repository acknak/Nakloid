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
    BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(path), fs::recursive_directory_iterator())) {
      if (p.leaf().string() == "oto.ini") {
        cout << "loading " << p.string() << endl;
        initVoiceMap(p.string());
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
    // read oto.ini
    Voice tmp_voice(this);
    vector<string> v1, v2;
    boost::algorithm::split(v1, buf, boost::is_any_of("="));
    boost::algorithm::split(v2, v1[1], boost::is_any_of(","));
    short tmp;
    tmp_voice.setWavPath(path_ini.parent_path().string()+"/"+v1[0]);
    tmp_voice.offs = (((tmp=boost::lexical_cast<double>(v2[1]))>0))?tmp:0;
    tmp_voice.cons = (((tmp=boost::lexical_cast<double>(v2[2]))>0))?tmp:0;
    tmp_voice.blnk = boost::lexical_cast<double>(v2[3]);
    tmp_voice.prec = boost::lexical_cast<double>(v2[4]);
    tmp_voice.ovrl = boost::lexical_cast<double>(v2[5]);
    tmp_voice.is_vcv = false;
    // sanitize
    if (tmp_voice.ovrl < 0) {
      tmp_voice.ovrl *= -1;
      tmp_voice.offs -= tmp_voice.ovrl;
      tmp_voice.cons += tmp_voice.ovrl;
      tmp_voice.prec += tmp_voice.ovrl;
      if (tmp_voice.blnk < 0) {
        tmp_voice.blnk -= tmp_voice.ovrl;
      }
    }
    if (tmp_voice.ovrl > tmp_voice.prec) {
      tmp_voice.prec = tmp_voice.ovrl;
    }
    if (tmp_voice.prec > tmp_voice.cons) {
      tmp_voice.cons = tmp_voice.prec;
    }
    if (tmp_voice.blnk<0 && tmp_voice.cons > -tmp_voice.blnk) {
      tmp_voice.blnk = -tmp_voice.cons;
    }
    // get Voice pron
    {
      string tmp_alias = (v2[0]=="")?tmp_voice.path_wav.stem().string():v2[0];
      tmp_voice.pron = tmp_voice.prefix = tmp_voice.suffix = "";
      // get prefix
      string::size_type pos_prefix = tmp_alias.find(" ");
      if (tmp_alias.size()>1 && pos_prefix != string::npos) {
        tmp_voice.prefix = tmp_alias.substr(0, pos_prefix+1);
        tmp_alias.erase(0, pos_prefix+1);
        if (tmp_voice.prefix!="- " && tmp_voice.prefix!="* ") {
          tmp_voice.is_vcv = true;
        }
      }
      // get suffix
      if (tmp_alias.size() > 2) {
        string tmp_suffix = tmp_alias.substr(tmp_alias.size()-2);
        if (isalpha(tmp_suffix[0]) && isdigit(tmp_suffix[1])) {
          tmp_voice.suffix = tmp_suffix;
          tmp_alias.erase(tmp_alias.size()-2, 2);
        }
      }
      tmp_voice.pron = tmp_alias;
    }
    // set vowel_map
    if (!tmp_voice.is_vcv) {
      map<string, string>::const_iterator it = nak::getVow2PronIt(tmp_voice.pron);
      if (it!=nak::vow2pron.end()) {
        WavParser wav_parser(tmp_voice.path_wav.string());
        wav_parser.addTargetTrack(0);
        if (wav_parser.parse()) {
          vector<double> tmp_wav = (*(wav_parser.getDataChunks().begin())).getDataVector();
          vector<double>::iterator it_tmp_wav_cons = tmp_wav.begin()+((tmp_voice.offs+tmp_voice.cons)/1000.0*wav_parser.getFormat().dwSamplesPerSec);
          vector<double>::iterator it_tmp_wav_min = it_tmp_wav_cons;
          short win_size = wav_parser.getFormat().dwSamplesPerSec / tmp_voice.getFrq();
          double tmp_min_rms = -1.0;
          for (int i=0; i<win_size*2; i++) {
            vector<double> tmp_wav(it_tmp_wav_cons+i-win_size, it_tmp_wav_cons+i+win_size);
            double tmp_rms = nak::getRMS(tmp_wav);
            if (tmp_rms<tmp_min_rms || tmp_min_rms<0) {
              tmp_min_rms = tmp_rms;
              it_tmp_wav_min = it_tmp_wav_cons+i;
            }
          }
          vowel_map[nak::pron2vow[it->second]+tmp_voice.suffix].assign(it_tmp_wav_min-win_size, it_tmp_wav_min+win_size);
        }
      }
    }
    voice_map[tmp_voice.prefix+tmp_voice.pron+tmp_voice.suffix] = tmp_voice;
  }
  return true;
}

const Voice* VoiceDB::getVoice(string alias)
{
  if (!isAlias(alias))
    return 0;

  return &voice_map[alias];
}

bool VoiceDB::isAlias(string alias)
{
  return !((voice_map.empty()&&!initVoiceMap()) || voice_map.count(alias)==0);
}

bool VoiceDB::isVowel(string alias)
{
  return vowel_map.count(alias)>0;
}

vector<double> VoiceDB::getVowel(string alias)
{
  return vowel_map[alias];
}

void VoiceDB::setSingerPath(string path_singer)
{
  this->path_singer = path_singer;
}

string VoiceDB::getSingerPath()
{
  return this->path_singer;
}
