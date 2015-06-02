#include "PitchmarkParameters.h"
using namespace std;
using namespace rapidjson;

void PitchmarkParameters::load(const boost::filesystem::wpath& path)
{
  boost::filesystem::wifstream ifs(path);
  wstring json((istreambuf_iterator<wchar_t>(ifs)), istreambuf_iterator<wchar_t>());
  GenericStringStream< UTF16<> > buffer(json.c_str());

  GenericDocument< UTF16<> > doc;
  doc.ParseStream(buffer);
  filename = doc[L"filename"].GetString();
  sub_fade_start = doc[L"sub_fade_start"].IsInt()?doc[L"sub_fade_start"].GetInt():0;
  base_pitch = doc[L"base_pitch"].IsInt() ? doc[L"base_pitch"].GetInt() : 0;
  {
    const GenericValue< UTF16<> >& tmp_val = doc[L"base_vowel_wav"];
    if (tmp_val.IsObject()) {
      if (tmp_val[L"filename"].IsString()) {
        base_vowel_wav_filename = tmp_val[L"filename"].GetString();
      }
      if (tmp_val[L"from"].IsInt()) {
        base_vowel_wav_from = tmp_val[L"from"].GetInt();
      }
      if (tmp_val[L"to"].IsInt()) {
        base_vowel_wav_from = tmp_val[L"to"].GetInt();
      }
    }
  }
  {
    const GenericValue< UTF16<> >& tmp_val = doc[L"prefix_vowel_wav"];
    if (tmp_val.IsObject()) {
      if (tmp_val[L"filename"].IsString()) {
        prefix_vowel_wav_filename = tmp_val[L"filename"].GetString();
      }
      if (tmp_val[L"from"].IsInt()) {
        prefix_vowel_wav_from = tmp_val[L"from"].GetInt();
      }
      if (tmp_val[L"to"].IsInt()) {
        prefix_vowel_wav_from = tmp_val[L"to"].GetInt();
      }
    }
  }
  {
    const GenericValue< UTF16<> >& tmp_val = doc[L"pitchmark_points"];
    if (tmp_val.IsArray()) {
      pitchmark_points.resize(tmp_val.Size(),0);
      for (SizeType i = 0; i < tmp_val.Size(); i++) {
        pitchmark_points[i] = tmp_val[i].IsInt()?tmp_val[i].GetInt():0;
      }
    }
  }
}

void PitchmarkParameters::save(const boost::filesystem::wpath& path)
{
  GenericDocument< UTF16<> > doc;
  Document::AllocatorType& allocator = doc.GetAllocator();
  doc.SetObject();
  doc.AddMember(L"filename", StringRef(filename.c_str()), allocator);
  doc.AddMember(L"sub_fade_start", sub_fade_start, allocator);
  doc.AddMember(L"base_pitch", base_pitch, allocator);
  {
    GenericValue< UTF16<> > base_vowel_wav(kObjectType);
    base_vowel_wav.AddMember(L"filename", StringRef(base_vowel_wav_filename.c_str()), allocator);
    base_vowel_wav.AddMember(L"from", base_vowel_wav_from, allocator);
    base_vowel_wav.AddMember(L"from", base_vowel_wav_to, allocator);
    doc.AddMember(L"base_vowel_wav", base_vowel_wav, allocator);
  }
  {
    GenericValue< UTF16<> > prefix_vowel_wav(kObjectType);
    prefix_vowel_wav.AddMember(L"filename", StringRef(prefix_vowel_wav_filename.c_str()), allocator);
    prefix_vowel_wav.AddMember(L"from", prefix_vowel_wav_from, allocator);
    prefix_vowel_wav.AddMember(L"from", prefix_vowel_wav_to, allocator);
    doc.AddMember(L"prefix_vowel_wav", prefix_vowel_wav, allocator);
  }
  {
    GenericValue< UTF16<> > tmp_val(kArrayType);
    for (size_t i=0; i<pitchmark_points.size(); i++) {
      tmp_val.PushBack(pitchmark_points[i], allocator);
    }
    doc.AddMember(L"pitchmark_points", tmp_val, allocator);
  }

  GenericStringBuffer< UTF16<> > buffer;
  PrettyWriter<GenericStringBuffer< UTF16<> >, UTF16<>, ASCII<> > writer(buffer);
  doc.Accept(writer);

  boost::filesystem::wofstream ofs(path, ios_base::trunc);
  ofs << buffer.GetString();
}
