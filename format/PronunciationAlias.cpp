#include "PronunciationAlias.h"

using namespace std;

map<wstring, wstring> PronunciationAlias::data_vowel2pron = boost::assign::map_list_of
  (L"a", L"あ")(L"i", L"い")(L"u", L"う")(L"e", L"え")(L"o", L"お")(L"n", L"ん");

map<wstring, wstring> PronunciationAlias::data_pron2vowel = boost::assign::map_list_of
  (L"あ", L"a")(L"い", L"i")(L"う", L"u")(L"え", L"e")(L"お", L"o")
  (L"ぁ", L"a")(L"ぃ", L"i")(L"ぅ", L"u")(L"ぇ", L"e")(L"ぉ", L"o")
  (L"か", L"a")(L"き", L"i")(L"く", L"u")(L"け", L"e")(L"こ", L"o")
  (L"が", L"a")(L"ぎ", L"i")(L"ぐ", L"u")(L"げ", L"e")(L"ご", L"o")
  (L"さ", L"a")(L"し", L"i")(L"す", L"u")(L"せ", L"e")(L"そ", L"o")
  (L"ざ", L"a")(L"じ", L"i")(L"ず", L"u")(L"ぜ", L"e")(L"ぞ", L"o")
  (L"た", L"a")(L"ち", L"i")(L"つ", L"u")(L"て", L"e")(L"と", L"o")
  (L"だ", L"a")(L"ぢ", L"i")(L"づ", L"u")(L"で", L"e")(L"ど", L"o")
  (L"な", L"a")(L"に", L"i")(L"ぬ", L"u")(L"ね", L"e")(L"の", L"o")
  (L"は", L"a")(L"ひ", L"i")(L"ふ", L"u")(L"へ", L"e")(L"ほ", L"o")
  (L"ば", L"a")(L"び", L"i")(L"ぶ", L"u")(L"べ", L"e")(L"ぼ", L"o")
  (L"ぱ", L"a")(L"ぴ", L"i")(L"ぷ", L"u")(L"ぺ", L"e")(L"ぽ", L"o")
  (L"ま", L"a")(L"み", L"i")(L"む", L"u")(L"め", L"e")(L"も", L"o")
  (L"や", L"a")(L"ゆ", L"u")(L"よ", L"o")(L"ゃ", L"a")(L"ゅ", L"u")(L"ょ", L"o")
  (L"ら", L"a")(L"り", L"i")(L"る", L"u")(L"れ", L"e")(L"ろ", L"o")
  (L"わ", L"a")(L"を", L"o")(L"ん", L"n")(L"ゐ", L"i")(L"ゑ", L"e")
  (L"ア", L"a")(L"イ", L"i")(L"ウ", L"u")(L"エ", L"e")(L"オ", L"o")(L"ヴ", L"u")
  (L"ァ", L"a")(L"ィ", L"i")(L"ゥ", L"u")(L"ェ", L"e")(L"ォ", L"o")
  (L"カ", L"a")(L"キ", L"i")(L"ク", L"u")(L"ケ", L"e")(L"コ", L"o")
  (L"ガ", L"a")(L"ギ", L"i")(L"グ", L"u")(L"ゲ", L"e")(L"ゴ", L"o")
  (L"サ", L"a")(L"シ", L"i")(L"ス", L"u")(L"セ", L"e")(L"ソ", L"o")
  (L"ザ", L"a")(L"ジ", L"i")(L"ズ", L"u")(L"ゼ", L"e")(L"ゾ", L"o")
  (L"タ", L"a")(L"チ", L"i")(L"ツ", L"u")(L"テ", L"e")(L"ト", L"o")
  (L"ダ", L"a")(L"ヂ", L"i")(L"ヅ", L"u")(L"デ", L"e")(L"ド", L"o")
  (L"ナ", L"a")(L"ニ", L"i")(L"ヌ", L"u")(L"ネ", L"e")(L"ノ", L"o")
  (L"ハ", L"a")(L"ヒ", L"i")(L"フ", L"u")(L"ヘ", L"e")(L"ホ", L"o")
  (L"バ", L"a")(L"ビ", L"i")(L"ブ", L"u")(L"ベ", L"e")(L"ボ", L"o")
  (L"パ", L"a")(L"ピ", L"i")(L"プ", L"u")(L"ペ", L"e")(L"ポ", L"o")
  (L"マ", L"a")(L"ミ", L"i")(L"ム", L"u")(L"メ", L"e")(L"モ", L"o")
  (L"ヤ", L"a")(L"ユ", L"u")(L"ヨ", L"o")(L"ャ", L"a")(L"ュ", L"u")(L"ｮ", L"o")
  (L"ラ", L"a")(L"リ", L"i")(L"ル", L"u")(L"レ", L"e")(L"ロ", L"o")
  (L"ワ", L"a")(L"ヲ", L"o")(L"ン", L"n")(L"ヰ", L"i")(L"ヱ", L"e");

PronunciationAlias::PronunciationAlias(const wstring& alias)
{
  pron = alias;
  wstring::size_type pos_prefix = alias.find(L" ");
  if (pron.size()>1 && pos_prefix != wstring::npos) {
    prefix = pron.substr(0, pos_prefix+1);
    pron.erase(0, pos_prefix+1);
  }
  if (pron.size() > 1) {
    for (size_t i=0; i<pron.size(); i++) {
      if (data_pron2vowel.count(pron.substr(pron.size()-1-i,1)) > 0) {
        suffix = pron.substr(pron.size()-i);
        pron.erase(pron.size()-i);
        break;
      }
    }
  }
}

bool PronunciationAlias::operator==(const PronunciationAlias& other) const
{
  return prefix==other.prefix && pron==other.pron && suffix==other.suffix;
}

bool PronunciationAlias::operator!=(const PronunciationAlias& other) const
{
  return prefix!=other.prefix || pron!=other.pron || suffix!=other.suffix;
}

bool PronunciationAlias::checkVCV() const
{
  return (!prefix.empty() && prefix.find(L" ")!=wstring::npos && (prefix[0]!=L'*'&&prefix[0]!=L'-'));
}

wstring PronunciationAlias::getAliasString() const
{
  return prefix + pron + suffix;
}

wstring PronunciationAlias::getPronVowel() const
{
  return pron2vowel(pron);
}

wstring PronunciationAlias::getPrefixVowel() const
{
  if (prefix.length() > 2) {
    return prefix.substr(prefix.length()-2, 1);
  }
  return L"";
}

bool PronunciationAlias::isVowel(wstring vowel)
{
  return data_vowel2pron.find(vowel) != data_vowel2pron.end();
}

wstring PronunciationAlias::vowel2pron(wstring vowel)
{
  if(data_vowel2pron.find(vowel) == data_vowel2pron.end()) {
    return L"";
  }
  return data_vowel2pron[vowel];
}

bool PronunciationAlias::isPron(wstring pron)
{
  return data_pron2vowel.find(pron) != data_pron2vowel.end();
}

wstring PronunciationAlias::pron2vowel(wstring pron)
{
  if(data_pron2vowel.find(pron) == data_pron2vowel.end()) {
    return L"";
  }
  return data_pron2vowel[pron];
}
