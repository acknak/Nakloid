#include "ScoreNAK.h"

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

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
  const GenericValue< UTF16<> >& tmp_notes = doc[L"Score"][L"Notes"];
  if (tmp_notes.IsArray()) {
    for (SizeType i = 0; i < tmp_notes.Size(); i++) {
      if (tmp_notes[i][L"id"].IsInt()) {
        Note tmp_note(this, tmp_notes[i][L"id"].GetInt());
        tmp_note.setPronAlias(tmp_notes[i][L"alias"].IsString()?tmp_notes[i][L"alias"].GetString():L"");
        tmp_note.setStart(tmp_notes[i][L"start"].IsInt()?tmp_notes[i][L"start"].GetInt():0);
        tmp_note.setEnd(tmp_notes[i][L"end"].IsInt()?tmp_notes[i][L"end"].GetInt():0);
        if (tmp_notes[i][L"margin"].IsArray()) {
          const GenericValue< UTF16<> >& tmp_margin = tmp_notes[i][L"margin"];
          tmp_note.setMargin(tmp_margin[SizeType(0)].IsInt()?tmp_margin[SizeType(0)].GetInt():0,
                             tmp_margin[SizeType(1)].IsInt()?tmp_margin[SizeType(1)].GetInt():0);
        }
        if (tmp_notes[i][L"padding"].IsArray()) {
          const GenericValue< UTF16<> >& tmp_padding = tmp_notes[i][L"padding"];
          tmp_note.setPadding(tmp_padding[SizeType(0)].IsInt()?tmp_padding[SizeType(0)].GetInt():0,
                             tmp_padding[SizeType(1)].IsInt()?tmp_padding[SizeType(1)].GetInt():0);
        }
        tmp_note.setPrec(tmp_notes[i][L"prec"].IsInt()?tmp_notes[i][L"prec"].GetInt():0);
        tmp_note.setOvrl(tmp_notes[i][L"ovrl"].IsInt()?tmp_notes[i][L"ovrl"].GetInt():0);
        tmp_note.setCons(tmp_notes[i][L"cons"].IsInt()?tmp_notes[i][L"cons"].GetInt():0);
        tmp_note.setBaseVelocity(tmp_notes[i][L"vel"].IsInt()?tmp_notes[i][L"vel"].GetInt():0);
        tmp_note.setBasePitch(tmp_notes[i][L"pitch"].IsInt()?tmp_notes[i][L"pitch"].GetInt():0);
        if (tmp_notes[i][L"vel_points"].IsArray()) {
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
