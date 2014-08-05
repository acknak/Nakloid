#include "ScoreNAK.h"

using namespace std;

ScoreNAK::ScoreNAK(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib, const boost::filesystem::path& path_song)
  :Score(path_score, vocal_lib, path_song){}

ScoreNAK::~ScoreNAK(){}

void ScoreNAK::load()
{
  clearNotes();

  wcout << L"nak: " << getScorePath() << endl;

  boost::filesystem::path boost_path_nak(getScorePath());
  boost::property_tree::wptree pt, pt_notes;
  boost::property_tree::read_json(boost_path_nak.string(), pt);
  BOOST_FOREACH (const boost::property_tree::wptree::value_type& child_notes, pt.get_child(L"Score.notes")) {
    Note *tmp_note;
    const boost::property_tree::wptree& pt_note = child_notes.second;
    if (boost::optional<long> id = pt_note.get_optional<long>(L"id"))
      tmp_note = new Note(this, id.get());
    else
      continue;
    if (boost::optional<wstring> alias = pt_note.get_optional<wstring>(L"alias"))
      tmp_note->setPronAlias(alias.get());
    if (boost::optional<bool> is_vcv = pt_note.get_optional<bool>(L"vcv"))
      tmp_note->isVCV(is_vcv);
    if (boost::optional<long> start = pt_note.get_optional<long>(L"start"))
      tmp_note->setStart(start.get());
    if (boost::optional<long> end = pt_note.get_optional<long>(L"end"))
      tmp_note->setEnd(end.get());
    if (boost::optional<wstring> margin = pt_note.get_optional<wstring>(L"margin")) {
      vector<wstring> v;
      boost::algorithm::split(v, margin.get(), boost::is_any_of(L","));
      if (v.size() == 2) {
        tmp_note->setMargin(boost::lexical_cast<short>(v[0]), boost::lexical_cast<short>(v[1]));
      }
    }
    if (boost::optional<wstring> padding = pt_note.get_optional<wstring>(L"padding")) {
      vector<wstring> v;
      boost::algorithm::split(v, padding.get(), boost::is_any_of(","));
      if (v.size() == 2) {
        tmp_note->setPadding(boost::lexical_cast<short>(v[0]), boost::lexical_cast<short>(v[1]));
      }
    }
    if (boost::optional<short> prec = pt_note.get_optional<short>(L"prec"))
      tmp_note->setPrec(prec.get());
    if (boost::optional<short> ovrl = pt_note.get_optional<short>(L"ovrl"))
      tmp_note->setOvrl(ovrl.get());
    if (boost::optional<short> cons = pt_note.get_optional<short>(L"cons"))
      tmp_note->setCons(cons.get());
    if (boost::optional<short> vel = pt_note.get_optional<short>(L"vel"))
      tmp_note->setBaseVelocity(vel.get());
    if (boost::optional<unsigned char> pitch = pt_note.get_optional<unsigned char>(L"pitch"))
      tmp_note->setBasePitch(pitch.get());
    BOOST_FOREACH (const boost::property_tree::wptree::value_type& child_vel_points, pt_note.get_child(L"vel_points")) {
      wstring ws = child_vel_points.second.get_value<wstring>();
      vector<wstring> v;
      boost::algorithm::split(v, ws, boost::is_any_of(L","));
      if (v.size() == 2)
        tmp_note->addVelocityPoint(boost::lexical_cast<long>(v[0]), boost::lexical_cast<short>(v[1]));
    }
    addNote(*tmp_note);
  }

  reloadPitches();
}
