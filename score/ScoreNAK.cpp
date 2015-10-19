#include "ScoreNAK.h"

using namespace std;
using namespace rapidjson;

ScoreNAK::ScoreNAK(const boost::filesystem::path& path_score, const VocalLibrary *vocal_lib, const boost::filesystem::path& path_song)
  :Score(path_score, vocal_lib, path_song){}

ScoreNAK::~ScoreNAK(){}

void ScoreNAK::load()
{
  clearNotes();

  wcout << L"nak: " << getScorePath() << endl;

  boost::filesystem::wifstream ifs(path_score);
  wstring json((istreambuf_iterator<wchar_t>(ifs)), istreambuf_iterator<wchar_t>());
  GenericStringStream< UTF16<> > buffer(json.c_str());

  GenericDocument< UTF16<> > doc;
  doc.ParseStream(buffer);
  if (!doc.HasMember(L"Score")) {
    cerr << "[ScoreNAK::load]" << getScorePath() << " is not nak score." << endl;
    return;
  }
  const GenericValue< UTF16<> >& tmp_score = doc[L"Score"];
  if (!tmp_score.HasMember(L"Notes")) {
    cerr << "[ScoreNAK::load]" << getScorePath() << " does not have Notes." << endl;
    return;
  }
  const GenericValue< UTF16<> >& tmp_notes = tmp_score[L"Notes"];
  if (tmp_notes.IsArray()) {
    for (SizeType i = 0; i < tmp_notes.Size(); i++) {
      if (tmp_notes[i][L"id"].IsInt()) {
        Note tmp_note(this, tmp_notes[i][L"id"].GetInt());
        if (tmp_notes[i].HasMember(L"alias") && tmp_notes[i][L"alias"].IsString()) {
          tmp_note.setPronAlias(tmp_notes[i][L"alias"].GetString());
        }
        if (tmp_notes[i].HasMember(L"start") && tmp_notes[i][L"start"].IsInt()) {
          tmp_note.setStart(tmp_notes[i][L"start"].GetInt());
        }
        if (tmp_notes[i].HasMember(L"end") && tmp_notes[i][L"end"].IsInt()) {
          tmp_note.setEnd(tmp_notes[i][L"end"].GetInt());
        }
        if (tmp_notes[i].HasMember(L"front_margin") && tmp_notes[i][L"front_margin"].IsInt()
          && tmp_notes[i].HasMember(L"back_margin") && tmp_notes[i][L"back_margin"].IsInt()) {
          tmp_note.setMargin(tmp_notes[i][L"front_margin"].GetInt(), tmp_notes[i][L"back_margin"].GetInt());
        }
        if (tmp_notes[i].HasMember(L"front_padding") && tmp_notes[i][L"front_padding"].IsInt()
          && tmp_notes[i].HasMember(L"back_padding") && tmp_notes[i][L"back_padding"].IsInt()) {
          tmp_note.setPadding(tmp_notes[i][L"front_padding"].GetInt(), tmp_notes[i][L"back_padding"].GetInt());
        }
        if (tmp_notes[i].HasMember(L"vel") && tmp_notes[i][L"vel"].IsInt()) {
          tmp_note.setBaseVelocity(tmp_notes[i][L"vel"].GetInt());
        }
        if (tmp_notes[i].HasMember(L"pitch") && tmp_notes[i][L"pitch"].IsInt()) {
          tmp_note.setBasePitch(tmp_notes[i][L"pitch"].GetInt());
        }
        if (tmp_notes[i].HasMember(L"vel_points") && tmp_notes[i][L"vel_points"].IsArray()) {
          const GenericValue< UTF16<> >& tmp_vel_points = tmp_notes[i][L"vel_points"];
          for (SizeType i = 0; i < tmp_vel_points.Size(); i++) {
            if (tmp_vel_points[L"vel_points"].IsArray()) {
              const GenericValue< UTF16<> >& tmp_vel_point = tmp_vel_points[i][L"vel_points"];
              tmp_note.addVelocityPoint(tmp_vel_point[SizeType(0)].IsInt()?tmp_vel_point[SizeType(0)].GetInt():0,
                                        tmp_vel_point[SizeType(1)].IsInt()?tmp_vel_point[SizeType(1)].GetInt():0);
            }
          }
        }
        addNote(tmp_note);
      }
    }
  }

  reloadPitches();
}
