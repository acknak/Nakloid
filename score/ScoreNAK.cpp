#include "ScoreNAK.h"

using namespace std;

ScoreNAK::ScoreNAK(string input_NAK, string path_pitches, string path_song, string path_singer)
  :Score(input_NAK, path_pitches, path_song, path_singer)
{
  load(input_NAK);
}

ScoreNAK::~ScoreNAK()
{
}

void ScoreNAK::load(string path_nak)
{
  cout << "----- start score(NAK) loading -----" << endl;

  boost::property_tree::wptree pt, pt_notes;
  boost::property_tree::read_json(path_nak, pt);
  BOOST_FOREACH (const boost::property_tree::wptree::value_type& child_notes, pt.get_child(L"Score.notes")) {
    Note *tmp_note;
    const boost::property_tree::wptree& pt_note = child_notes.second;
    if (boost::optional<long> id = pt_note.get_optional<long>(L"id"))
      tmp_note = new Note(this, id.get());
    else
      continue;
    if (boost::optional<wstring> pron_w = pt_note.get_optional<wstring>(L"pron")) {
      string pron;
	    char *mbs = new char[pron_w.get().length() * MB_CUR_MAX + 1];
	    wcstombs(mbs, pron_w.get().c_str(), pron_w.get().length() * MB_CUR_MAX + 1);
      tmp_note->setPron(mbs);
	    delete [] mbs;
    }
    if (boost::optional<long> start = pt_note.get_optional<long>(L"start"))
      tmp_note->setStart(start.get());
    if (boost::optional<long> end = pt_note.get_optional<long>(L"end"))
      tmp_note->setEnd(end.get());
    if (boost::optional<short> prec = pt_note.get_optional<short>(L"prec"))
      tmp_note->setPrec(prec.get());
    if (boost::optional<short> ovrl = pt_note.get_optional<short>(L"ovrl"))
      tmp_note->setOvrl(ovrl.get());
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
    notes.push_back(*tmp_note);
  }

  cout << "----- finish score(NAK) loading -----" << endl;
}
