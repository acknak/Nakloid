#include "ScoreUST.h"

using namespace std;

ScoreUST::ScoreUST(string input_ust, string path_pitches, string path_song, string path_singer)
  :Score(input_ust, path_pitches, path_song, path_singer),id_parse(0)
{
  load(input_ust);
}

ScoreUST::~ScoreUST(){}

void ScoreUST::load(string input_ust)
{
  cout << "----- start score(ust) loading -----" << endl
       << "ust: " << input_ust << endl;

  // read ust
  ifstream ifs(input_ust.c_str());
  string buf_str;
  list<string> buf_list;
  short tmp, tempo=120;
  unsigned long pos=0;
  notes.clear();
  while (ifs && getline(ifs, buf_str)) {
    if (buf_str == "[#SETTING]")
      continue;
    if (buf_str[0]=='[') {
      Note *tmp_note = new Note(this, ++id_parse);
      if (notes.size()>0) {
        tmp_note->setStart(notes.back().getEnd());
        if (notes.back().getPron()=="R")
          notes.pop_back();
      }
      notes.push_back(*tmp_note);
      continue;
    }
    vector<string> buf_vector;
    boost::algorithm::split(buf_vector, buf_str, boost::is_any_of("="));
    if (buf_vector[0] == "Tempo")
      tempo = (buf_vector[1]!="" && ((tmp=boost::lexical_cast<double>(buf_vector[1]))>0))?tmp:0;
    if (buf_vector[0]=="VoiceDir" && path_singer.empty()) {
      boost::algorithm::replace_all(buf_vector[1], "%", "/");
      if (buf_vector[1][0] != '/')
        buf_vector[1] = "/" + buf_vector[1];
      setSingerPath(".."+buf_vector[1]);
    }
    if (buf_vector[0] == "OutFile" && path_song.empty()) {
      boost::algorithm::replace_all(buf_vector[1], "%", "/");
      if (buf_vector[1][0] != '/')
        buf_vector[1] = "/" + buf_vector[1];
      setSongPath("."+buf_vector[1]);
    }
    if (buf_vector[0] == "Length")
      if (buf_vector[1]!="" && (tmp=boost::lexical_cast<double>(buf_vector[1]))>0)
        notes.back().setEnd(pos+=tmp, 480, 1.0/tempo*60000000);
    if (buf_vector[0] == "Lyric")
      notes.back().setPron(buf_vector[1]);
    if (buf_vector[0] == "NoteNum")
      if (buf_vector[1]!="" && (tmp=boost::lexical_cast<double>(buf_vector[1]))>0)
        notes.back().setBasePitch(tmp);
    if (buf_vector[0] == "PreUtterance")
      if (buf_vector[1]!="")
        notes.back().setPrec(boost::lexical_cast<double>(buf_vector[1]));
    if (buf_vector[0] == "VoiceOverlap")
      if (buf_vector[1]!="")
        notes.back().setOvrl(boost::lexical_cast<double>(buf_vector[1]));
    if (buf_vector[0] == "Intensity")
      if (buf_vector[1]!="" && (tmp=boost::lexical_cast<double>(buf_vector[1]))>0)
        notes.back().setBaseVelocity(tmp);
    if (buf_vector[0] == "Envelope") {
      vector<string> env_str_vector;
      vector<short> env_sht_vector;
      boost::algorithm::split(env_str_vector, buf_vector[1], boost::is_any_of(","));
      if (env_str_vector.size() >= 7) {
        for (int i=0; i<env_str_vector.size(); i++) {
          short tmp = 0;
          try {
            env_sht_vector.push_back(boost::lexical_cast<double>(env_str_vector[i]));
          } catch (...) {
            env_sht_vector.push_back(0);
          }
        }
        notes.back().addVelocityPoint(env_sht_vector[0], env_sht_vector[3]);
        notes.back().addVelocityPoint(env_sht_vector[0]+env_sht_vector[1], env_sht_vector[4]);
        if (env_str_vector.size() == 7) {
          notes.back().addVelocityPoint(-env_sht_vector[2], env_sht_vector[5]);
          notes.back().addVelocityPoint(-1, env_sht_vector[6]);
        } else if (env_str_vector.size() >= 9) {
          notes.back().addVelocityPoint(-env_sht_vector[2]-env_sht_vector[8], env_sht_vector[5]);
          notes.back().addVelocityPoint(-env_sht_vector[8], env_sht_vector[6]);
          if (env_str_vector.size() >= 11)
            notes.back().addVelocityPoint(env_sht_vector[0]+env_sht_vector[1]+env_sht_vector[9], env_sht_vector[10]);
        }
      }
    }
  }
  while (notes.back().getPron()=="R" || notes.back().getPron()=="")
    notes.pop_back();
  for (list<Note>::iterator it=notes.begin(); it!=notes.end(); ++it) {
    if (it->getVelocityPointNum() == 0) {
      it->addVelocityPoint(10, 100);
      it->addVelocityPoint(-35, 100);
    }
  }

  if (!is_tempered)
    reloadPitches();

  cout << "----- finish score(ust) loading -----" << endl;
}
