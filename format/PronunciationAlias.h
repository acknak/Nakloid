#ifndef PronunciationAlias_h
#define PronunciationAlias_h

#include <map>
#include <string>

class PronunciationAlias {
 public:
  PronunciationAlias():prefix(L""),pron(L""),suffix(L""){}
  explicit PronunciationAlias(const std::wstring& alias);
  PronunciationAlias(const std::wstring& prefix, const std::wstring& pron, const std::wstring& suffix):prefix(prefix),pron(pron),suffix(suffix){}
  bool operator==(const PronunciationAlias& other) const;
  bool operator!=(const PronunciationAlias& other) const;

  std::wstring prefix;
  std::wstring pron;
  std::wstring suffix;

  bool checkVCV() const;
  bool checkVowelPron() const;
  std::wstring getAliasString() const;
  std::wstring getPronVowel() const;
  std::wstring getPrefixVowel() const;
  static bool isVowel(std::wstring vowel);
  static bool isVowelPron(std::wstring pron);
  static bool isPron(std::wstring pron);
  static std::wstring vowel2pron(std::wstring vowel);
  static std::wstring pron2vowel(std::wstring pron);

 private:
  static std::map<std::wstring, std::wstring> data_vowel2pron;
  static std::map<std::wstring, std::wstring> data_pron2vowel;
};

#endif
