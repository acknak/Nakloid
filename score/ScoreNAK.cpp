#include "ScoreNAK.h"

using namespace std;

ScoreNAK::ScoreNAK(string input_NAK, string path_song, string path_singer)
  :Score(input_NAK, path_song, path_singer)
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
    if (boost::optional<wstring> alias_w = pt_note.get_optional<wstring>(L"alias")) {
	    char *mbs = new char[alias_w.get().length() * MB_CUR_MAX + 1];
	    wcstombs(mbs, alias_w.get().c_str(), alias_w.get().length() * MB_CUR_MAX + 1);
      tuple<string,string,string,bool> alias = nak::parseAlias(mbs);
	    delete [] mbs;
      tmp_note->setPrefix(get<0>(alias));
      tmp_note->setPron(get<1>(alias));
      tmp_note->setSuffix(get<2>(alias));
    }
    if (boost::optional<bool> is_vcv = pt_note.get_optional<bool>(L"vcv"))
      tmp_note->isVCV(is_vcv);
    if (boost::optional<long> start = pt_note.get_optional<long>(L"start"))
      tmp_note->setStart(start.get());
    if (boost::optional<long> end = pt_note.get_optional<long>(L"end"))
      tmp_note->setEnd(end.get());
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
    notes.push_back(*tmp_note);
  }

  if (!is_tempered)
    reloadPitches();

  cout << "----- finish score(NAK) loading -----" << endl;
}
