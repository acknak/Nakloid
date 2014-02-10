#include "ScoreUST.h"

using namespace std;

ScoreUST::ScoreUST(const wstring &path_score, const VoiceDB *voice_db, const std::wstring &path_song)
  :Score(path_score,voice_db,path_song),id_parse(0){};

ScoreUST::~ScoreUST(){}

void ScoreUST::load()
{
  clearNotes();

  wcout << L"ust: " << getScorePath() << endl;

  // read ust note
  boost::filesystem::wifstream ifs(getScorePath());
  wstring buf_str;
  vector< pair< short, vector<short> > > pitches_ust; //tempo & pitches
  long tempo=120, tmp_pos=0;
  Note *tmp_note = 0;
  pair< short, vector<short> > tmp_pitch;
  while (ifs && getline(ifs, buf_str)) {
    if (buf_str.size() < 3) {
      continue;
    }
    if (buf_str[0]=='[' && buf_str[1]=='#' && buf_str.back()==']') {
      // meta data
      if (tmp_note != 0) {
        if (tmp_note->getAlias().pron != L"R") {
          addNote(*tmp_note);
          tmp_pitch.first = tempo;
          pitches_ust.push_back(tmp_pitch);
        }
        delete tmp_note;
        tmp_note = 0;
        tmp_pitch = make_pair(0, vector<short>());
      }
      bool is_digit = true;
      for (size_t i=0; i<buf_str.size()-3; i++) {
        if (!iswdigit(buf_str[i+2])) {
          is_digit = false;
          break;
        }
      }
      if (is_digit) {
        tmp_note = new Note(this, ++id_parse);
        tmp_note->setStart(tmp_pos, 480, 1.0/tempo*60000000);
      }
    } else {
      // score parameter
      vector<wstring> buf_vector;
      long buf_long=0;
      boost::algorithm::split(buf_vector, buf_str, boost::is_any_of("="));
      if (buf_vector[0] == L"Tempo") {
        tempo = (buf_vector[1]!=L"" && ((buf_long=boost::lexical_cast<double>(buf_vector[1]))>0))?buf_long:0;
      } else if (tmp_note != 0) {
        // note parameter
        if (buf_vector[0] == L"Length") {
          if (!buf_vector[1].empty() && (buf_long=boost::lexical_cast<double>(buf_vector[1]))>0) {
            tmp_note->setEnd(tmp_pos+=buf_long, 480, 1.0/tempo*60000000);
          }
        } else if (buf_vector[0] == L"Lyric") {
          tmp_note->setAlias(buf_vector[1]);
          tmp_note->isVCV(tmp_note->getAlias().checkVCV());
        } else if (buf_vector[0] == L"NoteNum") {
          if (!buf_vector[1].empty() && (buf_long=boost::lexical_cast<double>(buf_vector[1]))>0) {
            tmp_note->setBasePitch(buf_long);
          }
        } else if (buf_vector[0] == L"PreUtterance") {
          if (!buf_vector[1].empty()) {
            tmp_note->setPrec(boost::lexical_cast<double>(buf_vector[1]));
          }
        } else if (buf_vector[0] == L"VoiceOverlap") {
          if (!buf_vector[1].empty()) {
            tmp_note->setOvrl(boost::lexical_cast<double>(buf_vector[1]));
          }
        } else if (buf_vector[0] == L"Intensity") {
          if (!buf_vector[1].empty() && (buf_long=boost::lexical_cast<double>(buf_vector[1]))>0) {
            tmp_note->setBaseVelocity(buf_long);
          }
        } else if (buf_vector[0] == L"Envelope") {
          vector<string> env_str_vector;
          vector<short> env_sht_vector;
          boost::algorithm::split(env_str_vector, buf_vector[1], boost::is_any_of(","));
          if (env_str_vector.size() >= 7) {
            for (size_t i=0; i<env_str_vector.size(); i++) {
              try {
                env_sht_vector.push_back(boost::lexical_cast<double>(env_str_vector[i]));
              } catch (...) {
                env_sht_vector.push_back(0);
              }
            }
            tmp_note->addVelocityPoint(env_sht_vector[0], env_sht_vector[3]);
            tmp_note->addVelocityPoint(env_sht_vector[0]+env_sht_vector[1], env_sht_vector[4]);
            if (env_str_vector.size() == 7) {
              tmp_note->addVelocityPoint(-env_sht_vector[2], env_sht_vector[5]);
              tmp_note->addVelocityPoint(-1, env_sht_vector[6]);
            } else if (env_str_vector.size() >= 9) {
              tmp_note->addVelocityPoint(-env_sht_vector[2]-env_sht_vector[8], env_sht_vector[5]);
              tmp_note->addVelocityPoint(-env_sht_vector[8], env_sht_vector[6]);
              if (env_str_vector.size() >= 11) {
                tmp_note->addVelocityPoint(env_sht_vector[0]+env_sht_vector[1]+env_sht_vector[9], env_sht_vector[10]);
              }
            }
          }
        } else if (buf_vector[0]==L"Pitches" || buf_vector[0]==L"Piches") {
          vector<string> pitch_str_vector;
          boost::algorithm::split(pitch_str_vector, buf_vector[1], boost::is_any_of(","));
          vector<short> pitch_sht_vector(pitch_str_vector.size(), 0);
          for (size_t i=0; i<pitch_str_vector.size(); i++) {
            try {
              pitch_sht_vector[i] = boost::lexical_cast<double>(pitch_str_vector[i]);
            } catch (...) {
              pitch_sht_vector[i] = 0;
            }
          }
          tmp_pitch.second = pitch_sht_vector;
        }
      }
    }
  }
  if (tmp_note!=0 && tmp_note->getAlias().pron!=L"R") {
    addNote(*tmp_note);
    tmp_pitch.first = tempo;
    pitches_ust.push_back(tmp_pitch);
    delete tmp_note;
    tmp_note = 0;
  }

  // get pitches
  reloadPitches();
  long tmp_length = 0;
  for (vector<Note>::const_iterator it=getNotesBegin(); it!=getNotesEnd(); ++it) {
    // search ms_end of song
    if (tmp_length < it->getPronEnd()) {
      tmp_length = it->getPronEnd();
    }
  }
  vector<float> tmp_pitches = getPitches();
  vector<Note>::const_iterator it_notes = getNotesBegin();
  vector< pair< short, vector<short> > >::iterator it_pitches = pitches_ust.begin();
  do {
    if (it_pitches->second.size() == 0) {
      continue;
    }
    long ms_note_start = it_notes->getPronStart()+it_notes->getFrontMargin();
    long ms_note_end = it_notes->getPronEnd();
    vector<short> note_velocities = it_notes->getVelocities();
    for (size_t i=0; i<it_pitches->second.size(); i++) {
      long ms_tick_start = ms_note_start + nak::tick2ms(i*5, 480, 1.0/it_pitches->first*60000000);
      long ms_tick_end = ms_note_start + nak::tick2ms((i+1)*5, 480, 1.0/it_pitches->first*60000000);
      if (ms_tick_end > ms_note_end) {
        ms_tick_end = ms_note_end;
      }
      if (ms_tick_end-ms_note_start >= note_velocities.size()) {
        ms_tick_end = ms_note_start + note_velocities.size() - 1;
      }
      float tmp_pitch = it_notes->getBasePitchHz()*pow(2.0,it_pitches->second[i]/1200.0);
      for (size_t j=0; j<ms_tick_end-ms_tick_start; j++) {
        long ms_tmp=ms_tick_start+j, ms_tmp_note=ms_tmp-ms_note_start;
        if (ms_tmp > 0) {
          tmp_pitches[ms_tmp] = tmp_pitch;
        }
      }
    }
  } while (++it_notes!=getNotesEnd() && ++it_pitches!=pitches_ust.end());
  setPitches(tmp_pitches);
}
