#include "Nakloid.h"

using namespace std;

Nakloid::Nakloid() : voice_db(0), margin(0), score(0)
{
  setDefaultFormat();
}

Nakloid::Nakloid(string path_ust) : voice_db(0), margin(0), score(0)
{
  setDefaultFormat();
  setScore(path_ust);
}

Nakloid::Nakloid(string singer, string path_smf, short track, string path_lyric, string path_song) : voice_db(0), margin(0), score(0)
{
  setDefaultFormat();
  setScore(singer, path_smf, track, path_lyric, path_song);
}

Nakloid::~Nakloid()
{
  if (voice_db != 0) {
    delete voice_db;
    voice_db = 0;
  }
  if (score != 0) {
    delete score;
    score = 0;
  }
}

void Nakloid::setDefaultFormat()
{
  format.chunkSize = 16;
  format.wFormatTag = 1;
  format.wChannels = 1;
  format.dwSamplesPerSec = 44100;
  format.dwAvgBytesPerSec = format.dwSamplesPerSec*2;
  format.wBlockAlign = 2;
  format.wBitsPerSamples = 16;
}

bool Nakloid::setScore(string path_ust)
{
  if (score != 0) {
    delete score;
    score = 0;
  }

  score = new Score(path_ust);
  voice_db = new VoiceDB(score->getSinger());

  return true;
}

bool Nakloid::setScore(string singer, string path_smf, short track, string path_lyric, string path_song)
{
  if (score != 0) {
    delete score;
    score = 0;
  }

  // get lyric
  ifstream ifs(path_lyric.c_str());
  string buf_str;
  list<string> prons;
  while (ifs && getline(ifs, buf_str)) {
    if (buf_str == "")
      continue;
    if (*(buf_str.end()-1) == ',')
      buf_str.erase(buf_str.end()-1,buf_str.end());
    vector<string> buf_vector;
    boost::algorithm::split(buf_vector, buf_str, boost::is_any_of(","));
    prons.insert(prons.end(), buf_vector.begin(), buf_vector.end());
  }

  // get score
  score = new Score(singer, path_smf, track, prons, path_song);
  if (score == 0 || !score->isScoreLoaded()) {
    cerr << "score hasn't loaded" << endl;
    return false;
  }

  // set voiceDB
  voice_db = new VoiceDB(score->getSinger());

  return true;
}

bool Nakloid::vocalization()
{
  if (score == 0) {
    cerr << "score hasn't loaded" << endl;
    return false;
  }

  if (voice_db == 0) {
    cerr << "can't find voiceDB" << endl;
    return false;
  }

  cout << "----- start vocalization -----" << endl;

  // load pitch_marks
  list<unsigned long> pitch_marks = getPitchMarks(score->getPitches());

  /*
  list<pitch_mark> pitch_marks;
  list<unsigned long> test;
  unsigned long tmp_ms = 0;
  while (++tmp_ms < pitches.size()) {
    if (pitches[tmp_ms] == 0)
      continue;
    pitch_mark tmp_pitch_mark;
    tmp_pitch_mark.ms_start = tmp_ms;
    tmp_pitch_mark.ms_end = (tmp_ms+=1000.0/pitches[tmp_ms]);
    test.push_back(tmp_pitch_mark.pos = (tmp_pitch_mark.ms_start+tmp_pitch_mark.ms_end)/2*(format.dwSamplesPerSec/1000.0));
    pitch_marks.push_back(tmp_pitch_mark);
  }
  */

  // overlap
  list<Note> notes = score->getNotesList();
  list<Note>::iterator it_notes = notes.begin();
  if (margin < max<unsigned long>(voice_db->getVoice(it_notes->getPron()).prec, it_notes->getStart()))
    margin = max<unsigned long>(voice_db->getVoice(it_notes->getPron()).prec, it_notes->getStart());
  vector<short> output_wav(ms2pos(score->getNotesList().back().getEnd()+margin), 0);

  do {
    Voice voice = voice_db->getVoice(it_notes->getPron());
    Voice voice_prev, voice_next;
    unsigned short prec = it_notes->isPrec()?it_notes->getPrec():voice.prec;
    unsigned short ovrl = it_notes->isOvrl()?it_notes->getOvrl():voice.ovrl;
    unsigned long pos_note_start = ms2pos(it_notes->getStart()-ovrl);

    list<unsigned long> tmp_pitch_marks(0);
    list<unsigned long>::iterator it_pitch_mark = pitch_marks.begin();
    do {
      if (pos2ms(*it_pitch_mark) < it_notes->getStart()-ovrl)
        continue;
      tmp_pitch_marks.push_back(*it_pitch_mark);
    } while (++it_pitch_mark!=pitch_marks.end() && pos2ms(*it_pitch_mark)<it_notes->getEnd());
    for (list<unsigned long>::reverse_iterator rit=tmp_pitch_marks.rbegin(); rit!=tmp_pitch_marks.rend(); ++rit)
      *rit -= tmp_pitch_marks.front();

    BaseWavsOverlapper *overlapper = new BaseWavsOverlapper();
    overlapper->setPitchMarks(tmp_pitch_marks);
    overlapper->setBaseWavs(voice.bwc.data);
    overlapper->setRepStart(voice.bwc.format.dwRepeatStart);
    overlapper->setVelocity(it_notes->getVelocity());
    overlapper->overlapping();
    vector<short> note_wav = overlapper->getOutputWavVector();
    delete overlapper;

    DataArranger::edge_back(&note_wav, format.dwSamplesPerSec);

    for (int i=0; i<note_wav.size(); i++)
      output_wav[i+pos_note_start] += note_wav[i];
  } while (++it_notes != notes.end());

  long size = output_wav.size()*sizeof(short);
  ofstream ofs(score->getSongPath(), ios_base::out|ios_base::trunc|ios_base::binary);
  WavParser::setWavHeader(&ofs, format, size+28);
  ofs.write((char*)WavFormat::data, sizeof(char)*4);
  ofs.write((char*)&size, sizeof(long));
  ofs.write((char*)(&output_wav[0]), size);
  ofs.close();

  cout << "----- vocalization finished -----" << endl << endl;
  return true;
}

WavFormat Nakloid::getFormat()
{
  return format;
}

void Nakloid::setFormat(WavFormat format)
{
  this->format = format;
}

Score* Nakloid::getScore()
{
  return score;
}

void Nakloid::setMargin(long margin)
{
  this->margin = margin;
}

long Nakloid::getMargin()
{
  return this->margin;
}

list<unsigned long> Nakloid::getPitchMarks(vector<double> pitches)
{
  unsigned long tmp_ms = 0;
  list<unsigned long> pitch_marks(0);
  bool is_note_on = false;

  while (tmp_ms < pitches.size()) {
    if (pitches[tmp_ms] > 0) {
      if (!is_note_on)
        pitch_marks.push_back(ms2pos(tmp_ms));
      pitch_marks.push_back(pitch_marks.back()+(1.0/pitches[tmp_ms]*format.dwSamplesPerSec));
      tmp_ms = pos2ms(pitch_marks.back());
      is_note_on = true;
    } else {
      tmp_ms++;
      is_note_on = false;
    }
  }

  return pitch_marks;
}

unsigned long Nakloid::ms2pos(unsigned long ms)
{
  return ms / 1000.0 * format.dwSamplesPerSec;
}

unsigned long Nakloid::pos2ms(unsigned long pos)
{
  return pos / (double)format.dwSamplesPerSec * 1000;
}
