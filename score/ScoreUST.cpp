#include "ScoreUST.h"

using namespace std;

ScoreUST::ScoreUST(string input_ust, string path_song, string path_singer)
  :Score(input_ust, path_song, path_singer),id_parse(0)
{
  load(input_ust);
}

ScoreUST::~ScoreUST(){}

void ScoreUST::load(string input_ust)
{
  cout << "----- start score(ust) loading -----" << endl
       << "ust: " << input_ust << endl;

  // read ust note
  ifstream ifs(input_ust.c_str());
  string buf_str;
  list<string> buf_list;
  list< pair< short, vector<short> > > pitches_ust;
  short tmp, tempo=120;
  unsigned long pos = 0;
  bool is_parse = false;
  notes.clear();
  while (ifs && getline(ifs, buf_str)) {
    if (buf_str[0]=='[') {
      if (buf_str.size() != 7) {
        is_parse = false;
        continue;
      } else {
        try {
          string tmp(buf_str, 2, 4);
          boost::lexical_cast<short>(tmp);
          is_parse = true;
        } catch (...) {
          is_parse = false;
          continue;
        }
      }

      Note *tmp_note = new Note(this, ++id_parse);
      if (notes.size()>0) {
        tmp_note->setStart(notes.back().getEnd());
        if (notes.back().getPron()=="R") {
          notes.pop_back();
          pitches_ust.pop_back();
        }
      }
      notes.push_back(*tmp_note);
      vector<short> tmp_vector;
      pitches_ust.push_back(make_pair(0, tmp_vector));
      is_parse = true;
      continue;
    } else if (!is_parse) {
      continue;
    }

    vector<string> buf_vector;
    boost::algorithm::split(buf_vector, buf_str, boost::is_any_of("="));
    if (buf_vector[0] == "Tempo") {
      tempo = (buf_vector[1]!="" && ((tmp=boost::lexical_cast<double>(buf_vector[1]))>0))?tmp:0;
    } else if (buf_vector[0]=="VoiceDir" && path_singer.empty()) {
      boost::algorithm::replace_all(buf_vector[1], "%", "/");
      if (buf_vector[1][0] != '/')
        buf_vector[1] = "/" + buf_vector[1];
      setSingerPath(".."+buf_vector[1]);
    } else if (buf_vector[0] == "OutFile" && path_song.empty()) {
      boost::algorithm::replace_all(buf_vector[1], "%", "/");
      if (buf_vector[1][0] != '/')
        buf_vector[1] = "/" + buf_vector[1];
      setSongPath("."+buf_vector[1]);
    } else if (buf_vector[0] == "Length") {
      if (buf_vector[1]!="" && (tmp=boost::lexical_cast<double>(buf_vector[1]))>0)
        notes.back().setEnd(pos+=tmp, 480, 1.0/tempo*60000000);
    } else if (buf_vector[0] == "Lyric") {
      notes.back().setPron(buf_vector[1]);
    } else if (buf_vector[0] == "NoteNum") {
      if (buf_vector[1]!="" && (tmp=boost::lexical_cast<double>(buf_vector[1]))>0)
        notes.back().setBasePitch(tmp);
    } else if (buf_vector[0] == "PreUtterance") {
      if (buf_vector[1]!="")
        notes.back().setPrec(boost::lexical_cast<double>(buf_vector[1]));
    } else if (buf_vector[0] == "VoiceOverlap") {
      if (buf_vector[1]!="")
        notes.back().setOvrl(boost::lexical_cast<double>(buf_vector[1]));
    } else if (buf_vector[0] == "Intensity") {
      if (buf_vector[1]!="" && (tmp=boost::lexical_cast<double>(buf_vector[1]))>0)
        notes.back().setBaseVelocity(tmp);
    } else if (buf_vector[0] == "Envelope") {
      vector<string> env_str_vector;
      vector<short> env_sht_vector;
      boost::algorithm::split(env_str_vector, buf_vector[1], boost::is_any_of(","));
      if (env_str_vector.size() >= 7) {
        for (int i=0; i<env_str_vector.size(); i++)
          try {
            env_sht_vector.push_back(boost::lexical_cast<double>(env_str_vector[i]));
          } catch (...) {
            env_sht_vector.push_back(0);
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
    } else if (buf_vector[0]=="Pitches" || buf_vector[0]=="Piches") {
      vector<string> pitch_str_vector;
      boost::algorithm::split(pitch_str_vector, buf_vector[1], boost::is_any_of(","));
      vector<short> pitch_sht_vector(pitch_str_vector.size(), 0);
      for (int i=0; i<pitch_str_vector.size(); i++) {
        try {
          pitch_sht_vector[i] = boost::lexical_cast<double>(pitch_str_vector[i]);
        } catch (...) {
          pitch_sht_vector[i] = 0;
        }
      }
      pitches_ust.pop_back();
      pitches_ust.push_back(make_pair(tempo, pitch_sht_vector));
    }
  }
  while (notes.back().getPron()=="R" || notes.back().getPron()=="") {
    notes.pop_back();
    pitches_ust.pop_back();
  }
  for (list<Note>::iterator it=notes.begin(); it!=notes.end(); ++it) {
    if (it->getVelocityPointNum() == 0) {
      it->addVelocityPoint(10, 100);
      it->addVelocityPoint(-35, 100);
    }
  }

  // get pitches
  if (!is_tempered) {
    long tmp_length = 0;
    for (list<Note>::iterator it=notes.begin(); it!=notes.end(); ++it)
      if (tmp_length < it->getPronEnd())
        tmp_length = it->getPronEnd();
    pitches.assign(tmp_length, 0);
    vector<long> velocities(tmp_length, 0);
    list<Note>::iterator it_notes = notes.begin();
    list< pair< short, vector<short> > >::iterator it_pitches = pitches_ust.begin();
    do {
      vector<short> note_velocities = it_notes->getVelocities();
      long note_start_ms = it_notes->getPronStart();
      long note_end_ms = it_notes->getPronEnd();
      if (it_pitches->second.size() == 0)
        it_pitches->second.push_back(0);
      for (int i=0; i<it_pitches->second.size(); i++) {
        long tick_start_ms = note_start_ms + nak::tick2ms(i*5, 480, 1.0/it_pitches->first*60000000);
        long tick_end_ms = note_start_ms + nak::tick2ms((i+1)*5, 480, 1.0/it_pitches->first*60000000);
        if (tick_end_ms > note_end_ms)
          tick_end_ms = note_end_ms;
        if (tick_end_ms-note_start_ms >= note_velocities.size())
          tick_end_ms = note_start_ms + note_velocities.size() - 1;
        double tmp_pitch = it_notes->getBasePitchHz()*pow(2.0,it_pitches->second[i]/1200.0);
        if (i == it_pitches->second.size()-1) {
          for (int j=tick_end_ms; j<note_end_ms; j++) {
            pitches[j] = tmp_pitch;
            velocities[j] = note_velocities[j-tick_end_ms];
          }
        }
        for (int j=0; j<tick_end_ms-tick_start_ms; j++) {
          long tmp_ms=tick_start_ms+j, tmp_note_ms=tmp_ms-note_start_ms;
          if (velocities[tmp_ms] == 0)
            pitches[tmp_ms] = tmp_pitch;
          else {
            pitches[tmp_ms] =
              (pitches[tmp_ms]/(velocities[tmp_ms]+note_velocities[tmp_note_ms])*velocities[tmp_ms])
              + (tmp_pitch/(velocities[tmp_ms]+note_velocities[tmp_note_ms])*note_velocities[tmp_note_ms]);
          }
          velocities[tmp_ms] += note_velocities[tmp_note_ms];
        }
      }
    } while (++it_notes!=notes.end() && ++it_pitches!=pitches_ust.end());
  }

  cout << "----- finish score(ust) loading -----" << endl;
}
