#include "Arranger.h"

using namespace std;

VoiceDB* Arranger::voice_db = 0;
Score* Arranger::score = 0;

void Arranger::arrange(VoiceDB *voice_db, Score *score)
{
  Arranger::voice_db = voice_db;
  Arranger::score = score;

  cout << "set note params..." << endl;
  for (list<Note>::iterator it_notes=score->notes.begin(); it_notes!=score->notes.end(); ++it_notes) {
    checkAlias(it_notes);
    if (voice_db->getVoice(it_notes->getAlias()) == 0) {
      continue;
    }
    loadParamsFromVoiceDB(it_notes, voice_db->getVoice(it_notes->getAlias()));
  }

  cout << "arrange pitch params..." << endl;
  vector<float> pitches = score->getPitches();
  for (list<Note>::iterator it_notes=score->notes.begin();it_notes!=score->notes.end();++it_notes) {
    if (nak::vibrato)
      vibrato(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd());
    if (nak::interpolation)
      interpolation(pitches.begin(), it_notes->getPronStart(), it_notes->getPronEnd(), it_notes->getBasePitchHz());
    if (nak::overshoot)
      if (it_notes!=score->notes.begin() && it_notes->getStart()==boost::prior(it_notes)->getEnd())
        overshoot(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd(), *(pitches.begin()+boost::prior(it_notes)->getEnd()-1));
    if (nak::preparation)
      if (it_notes!=--score->notes.end() && it_notes->getEnd()==boost::next(it_notes)->getStart())
        preparation(pitches.begin()+it_notes->getStart(), pitches.begin()+it_notes->getEnd(), *(pitches.begin()+boost::next(it_notes)->getStart()));
  }
  if (nak::finefluctuation) {
    boost::minstd_rand gen;
    boost::normal_distribution<> dst( 0.0, nak::finefluctuation_deviation );
	  boost::variate_generator<boost::minstd_rand&, boost::normal_distribution<>> rand( gen, dst );
    for (vector<float>::iterator it_pitch=pitches.begin();it_pitch!=pitches.end();++it_pitch) {
      if (*it_pitch > 0)
        *it_pitch += rand();
    }
  }

  cout << endl;
  score->setPitches(pitches);
}

void Arranger::checkAlias(list<Note>::iterator it_notes)
{
  if (nak::path_prefix_map != "") {
    // add prefix & suffix
    pair<string, string> tmp_modifier = score->getModifier(it_notes->getBasePitch());
    it_notes->setPrefix(tmp_modifier.first + it_notes->getPrefix());
    it_notes->setSuffix(it_notes->getSuffix() + tmp_modifier.second);
  }
  if (nak::auto_vowel_combining && it_notes!=score->notes.begin()
    && boost::prior(it_notes)->getEnd()==it_notes->getStart()
    && (it_notes->getPrefix()=="*"||it_notes->getPrefix().empty())
    && voice_db->isAlias("* "+it_notes->getPron())) {
    // vowel combining
    it_notes->setPrefix("* ");
  }
  if (it_notes->getPrefix() == "* ") {
    it_notes->setBaseVelocity(it_notes->getBaseVelocity()*nak::vowel_combining_volume);
  }
  if (!voice_db->isAlias(it_notes->getAlias())) {
    if (it_notes->getPron().find("を")!=string::npos
      && voice_db->isAlias(boost::algorithm::replace_all_copy(it_notes->getPron(), "を", "お"))) {
      // "wo" to "o"
      it_notes->setPron(boost::algorithm::replace_all_copy(it_notes->getPron(), "を", "お"));
    } else {
      cerr << "[Nakloid::vocalization] can't find pron: \"" << it_notes->getAlias() << "\"" << endl;
    }
  }
}

void Arranger::loadParamsFromVoiceDB(list<Note>::iterator it_notes, const Voice* voice)
{
  it_notes->isVCV(voice->is_vcv||it_notes->isVCV());
  if (!it_notes->isOvrl())
    it_notes->setOvrl(voice->ovrl);
  if (!it_notes->isPrec())
    it_notes->setPrec(voice->prec);
}

void Arranger::vibrato(vector<float>::iterator it_pitches_begin, vector<float>::iterator it_pitches_end)
{
  if ((it_pitches_end-it_pitches_begin) > nak::ms_vibrato_offset) {
    unsigned long vibrato_length = (it_pitches_end-it_pitches_begin) - nak::ms_vibrato_offset;
    for (unsigned long i=0; i<vibrato_length; i++)
      *(it_pitches_begin+nak::ms_vibrato_offset+i) += sin(2*M_PI*i/nak::ms_vibrato_width) * nak::pitch_vibrato;
  }
}

void Arranger::overshoot(vector<float>::iterator it_pitches_begin, vector<float>::iterator it_pitches_end, float target_pitch)
{
  float diff = (*it_pitches_begin-target_pitch);

  if (it_pitches_end-it_pitches_begin > nak::ms_overshoot)
    for (int i=0; i<nak::ms_overshoot/2; i++) {
      *(it_pitches_begin+i) +=
        -diff + ((diff+(nak::pitch_overshoot*((diff>0)?1:-1))) / (nak::ms_overshoot/2) * i);
      *(it_pitches_begin+(nak::ms_overshoot/2)+i) +=
        (nak::pitch_overshoot*((diff>0)?1:-1)) + ((nak::pitch_overshoot*((diff>0)?-1:1))/(nak::ms_overshoot/2) * i);
    }
  else
    for (int i=0; i<it_pitches_end-it_pitches_begin; i++)
      *(it_pitches_begin+i) += -diff + (diff/(it_pitches_end-it_pitches_begin)*i);
}

void Arranger::preparation(vector<float>::iterator it_pitches_begin, vector<float>::iterator it_pitches_end, float target_pitch)
{
  vector<float>::reverse_iterator rit_pitches_begin(it_pitches_end);
  vector<float>::reverse_iterator rit_pitches_end(it_pitches_begin);
  float diff = (*rit_pitches_begin-target_pitch) / 2;

  if (rit_pitches_end-rit_pitches_begin > nak::ms_preparation)
    for (int i=0; i<nak::ms_preparation/2; i++) {
      *(rit_pitches_begin+i) +=
        -diff + ((diff+(nak::pitch_preparation*((diff>0)?1:-1))) / (nak::ms_preparation/2) * i);
      *(rit_pitches_begin+(nak::ms_preparation/2)+i) +=
        (nak::pitch_preparation*((diff>0)?1:-1)) + ((nak::pitch_preparation*((diff>0)?-1:1))/(nak::ms_preparation/2) * i);
    }
  else
    for (int i=0; i<rit_pitches_end-rit_pitches_begin; i++)
      *(rit_pitches_begin+i) += -diff + (diff/(rit_pitches_end-rit_pitches_begin)*i);
}

void Arranger::interpolation(vector<float>::iterator it_pitches, unsigned long ms_pron_start, unsigned long ms_pron_end, float target_pitch)
{
  for (unsigned long i=0; i<ms_pron_end-ms_pron_start; i++)
    if (*(it_pitches+ms_pron_start+i) == 0)
      *(it_pitches+ms_pron_start+i) = target_pitch;
}
